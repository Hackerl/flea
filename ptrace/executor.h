#ifndef FLEA_EXECUTOR_H
#define FLEA_EXECUTOR_H

#include "tracee.h"

enum emState {
    DETACHED,
    ATTACHED,
    TERMINATED
};

class CExecutor : public CTracee {
public:
    explicit CExecutor(pid_t pid);

public:
    bool run(const unsigned char *shellcode, unsigned int length, void *base, void *stack, void *argument, int &status);
    bool call(const unsigned char *shellcode, unsigned int length, void *base, void *stack, void *argument, void **result);

private:
    bool getExecBase(void **base) const;
};


#endif //FLEA_EXECUTOR_H
