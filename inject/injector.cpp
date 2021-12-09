#include "injector.h"
#include <zero/log.h>
#include <zero/proc/process.h>
#include <zero/filesystem/path.h>
#include <shellcode/alloc.h>
#include <shellcode/free.h>
#include <shellcode/loader.h>
#include <shellcode/loader/payload.h>
#include <elfio/elfio.hpp>
#include <unistd.h>

constexpr auto ALLOC_SIZE = 0x21000;

constexpr auto LIBC_IMAGE = "libc-";
constexpr auto DLOPEN_SYMBOL = "__libc_dlopen_mode";

CInjector::CInjector(pid_t pid) {
    mPID = pid;
    mPageSize = sysconf(_SC_PAGESIZE);
}

CInjector::~CInjector() {
    for (const auto &executor : mExecutors) {
        executor->detach();
        delete executor;
    }
}

bool CInjector::open() {
    std::list<pid_t> threads;

    if (!zero::proc::getThreads(mPID, threads)) {
        LOG_ERROR("get process %d threads failed", mPID);
        return false;
    }

    for (const auto &tid : threads) {
        std::unique_ptr<CExecutor> executor(new CExecutor(tid));

        if (!executor->attach())
            return false;

        mExecutors.push_back(executor.release());
    }

    LOG_INFO("attach process %d success", mPID);

    return true;
}

int CInjector::inject(const std::string &library) {
    if (library.size() >= sizeof(loader_payload_t::library)) {
        LOG_ERROR("payload size limit");
        return -1;
    }

    loader_payload_t payload = {};

    memcpy(payload.library, library.data(), library.size());

    if (!getDlopenAddress((void **)&payload.dlopen_mode))
        return -1;

    void *result = nullptr;
    CExecutor *executor = mExecutors.front();

    LOG_INFO("execute alloc shellcode");

    if (!executor->call(alloc_sc, alloc_sc_len, nullptr, nullptr, nullptr, &result)) {
        LOG_ERROR("execute alloc shellcode failed");
        return -1;
    }

    if (!result) {
        LOG_INFO("allocate memory failed");
        return -1;
    }

    LOG_INFO("memory allocated: %p", result);

    if (!executor->getRegisters(payload.regs)) {
        LOG_ERROR("get registers failed");
        return -1;
    }

    if (!executor->writeMemory(result, &payload, sizeof(payload))) {
        LOG_ERROR("write loader payload failed");
        return -1;
    }

    int status = 0;

    unsigned long base = ((unsigned long)result + sizeof(payload) + mPageSize - 1) & ~(mPageSize - 1);
    unsigned long stack = (unsigned long)result + ALLOC_SIZE;

    LOG_INFO("execute loader shellcode");

    if (!executor->run(loader_sc, loader_sc_len, (void *)base, (void *)stack, result, status)) {
        LOG_ERROR("execute loader shellcode failed");
        return -1;
    }

    LOG_INFO("execute free shellcode");

    if (!executor->call(free_sc, free_sc_len, nullptr, nullptr, result, nullptr)) {
        LOG_ERROR("execute free shellcode failed");
        return -1;
    }

    return status;
}

bool CInjector::getDlopenAddress(void **address) const {
    zero::proc::CProcessMapping processMapping;

    if (!zero::proc::getImageBase(mPID, LIBC_IMAGE, processMapping)) {
        LOG_ERROR("can't find libc image base");
        return false;
    }

    LOG_INFO("libc image base: 0x%lx", processMapping.start);

    std::string path = zero::filesystem::path::join("/proc", std::to_string(mPID), "root", processMapping.pathname);

    ELFIO::elfio reader;

    if (!reader.load(path)) {
        LOG_ERROR("open elf failed: %s", path.c_str());
        return false;
    }

    auto it = std::find_if(
            reader.sections.begin(),
            reader.sections.end(),
            [](const auto& s) {
                return s->get_type() == SHT_DYNSYM;
            });

    if (it == reader.sections.end()) {
        LOG_ERROR("can't find symbol section");
        return false;
    }

    unsigned long baseAddress = 0;

    if (reader.get_type() != ET_EXEC) {
        std::vector<ELFIO::segment *> loads;

        std::copy_if(
                reader.segments.begin(),
                reader.segments.end(),
                std::back_inserter(loads),
                [](const auto &i){
                    return i->get_type() == PT_LOAD;
                });

        auto minElement = std::min_element(
                loads.begin(),
                loads.end(),
                [](const auto &i, const auto &j) {
                    return i->get_virtual_address() < j->get_virtual_address();
                });

        baseAddress = processMapping.start - ((*minElement)->get_virtual_address() & ~(mPageSize - 1));
    }

    ELFIO::Elf64_Addr dlopenAddress = 0;
    ELFIO::symbol_section_accessor symbols(reader, *it);

    for (ELFIO::Elf_Xword i = 0; i < symbols.get_symbols_num(); i++) {
        std::string name;
        ELFIO::Elf64_Addr value = 0;
        ELFIO::Elf_Xword size = 0;
        unsigned char bind = 0;
        unsigned char type = 0;
        ELFIO::Elf_Half section = 0;
        unsigned char other = 0;

        if (!symbols.get_symbol(i, name, value, size, bind, type, section, other)) {
            LOG_ERROR("get symbol %lu failed", i);
            return false;
        }

        if (name == DLOPEN_SYMBOL) {
            dlopenAddress = baseAddress + value;
            break;
        }
    }

    LOG_INFO("dlopen address: 0x%lx", dlopenAddress);

    if (!dlopenAddress) {
        LOG_ERROR("can't find dlopen symbol");
        return false;
    }

    *address = (void *)dlopenAddress;

    return true;
}
