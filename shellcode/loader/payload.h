#ifndef FLEA_PAYLOAD_H
#define FLEA_PAYLOAD_H

#include "quit.h"

typedef void *(*dlopen_mode_ptr)(const char *name, int mode);

typedef struct {
    char library[1024];
    dlopen_mode_ptr dlopen_mode;
    regs_t regs;
} loader_payload_t;

#endif //FLEA_PAYLOAD_H
