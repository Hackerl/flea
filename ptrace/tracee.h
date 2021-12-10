#ifndef FLEA_TRACEE_H
#define FLEA_TRACEE_H

#include <set>
#include <sys/user.h>
#include <sys/types.h>

#ifdef __arm__
typedef user_regs regs_t;
typedef user_fpregs fp_regs_t;
#elif __aarch64__
typedef user_regs_struct regs_t;
typedef user_fpsimd_struct fp_regs_t;
#else
typedef user_regs_struct regs_t;
typedef user_fpregs_struct fp_regs_t;
#endif

class CTracee {
public:
    explicit CTracee(pid_t pid);

public:
    bool attach() const;
    bool detach() const;

public:
    bool resume(int sig) const;
    bool catchSyscall(int sig) const;

public:
    bool getRegisters(regs_t &regs) const;
    bool setRegisters(regs_t &regs) const;

public:
    bool getFPRegisters(fp_regs_t &fp_regs) const;
    bool setFPRegisters(fp_regs_t &fp_regs) const;

public:
    bool readMemory(void *address, void *buffer, unsigned long length) const;
    bool writeMemory(void *address, void *buffer, unsigned long length) const;

public:
    bool setSyscall(long number) const;

protected:
    pid_t mPID;
};


#endif //FLEA_TRACEE_H
