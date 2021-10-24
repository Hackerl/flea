#include "inject/injector.h"
#include <zero/log.h>
#include <zero/cmdline.h>

int main(int argc, char ** argv) {
    INIT_CONSOLE_LOG(zero::INFO);

    zero::CCmdline cmdline;

    cmdline.add({"pid", "process id", zero::value<int>()});
    cmdline.add({"library", "shared library", zero::value<std::string>()});

    cmdline.parse(argc, argv);

    int pid = cmdline.get<int>("pid");
    std::string library = cmdline.get<std::string>("library");

    LOG_INFO("load %s", library.c_str());

    CInjector injector(pid);

    if (!injector.open()) {
        LOG_ERROR("process injector open failed");
        return -1;
    }

    return injector.inject(library);
}
