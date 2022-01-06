#ifndef FLEA_EXECUTOR_H
#define FLEA_EXECUTOR_H

#include "tracee.h"
#include <string>
#include <list>

enum emState {
    DETACHED,
    ATTACHED,
    TERMINATED
};

struct CFrame {
    std::string name;
    unsigned long offset;
};

class CExecutor : public CTracee {
public:
    explicit CExecutor(pid_t pid);

public:
    bool run(const unsigned char *shellcode, unsigned int length, void *base, void *stack, void *argument, int &status);
    bool call(const unsigned char *shellcode, unsigned int length, void *base, void *stack, void *argument, void **result);

public:
    bool getStackTrace(std::list<CFrame> &stackTrace);

private:
    bool getExecBase(void **base) const;
};


#endif //FLEA_EXECUTOR_H
