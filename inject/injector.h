#ifndef FLEA_INJECTOR_H
#define FLEA_INJECTOR_H

#include <ptrace/executor.h>
#include <list>
#include <vector>
#include <string>

class CInjector {
public:
    explicit CInjector(pid_t pid);
    ~CInjector();

public:
    bool open();

public:
    int inject(const std::string &library);

private:
    bool getDlopenAddress(void **address) const;

private:
    pid_t mPID;
    unsigned long mPageSize;

private:
    std::list<CExecutor *> mExecutors;
};


#endif //FLEA_INJECTOR_H
