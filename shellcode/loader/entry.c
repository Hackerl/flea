#include "payload.h"
#include <z_log.h>
#include <dlfcn.h>

void main(void *ptr) {
    loader_payload_t *payload = (loader_payload_t *)ptr;
    snapshot(&payload->context);

    LOG("load library: %s", payload->library);

    if (!payload->dlopen_mode(payload->library, RTLD_LAZY)) {
        LOG("load library failed");
        quit(-1);
    }

    quit(0);
}

#if __i386__ || __x86_64__

__asm__ (
".section .entry;"
".global entry;"
"entry:"
"    nop;"
"    nop;"
"    call main;"
"    int3"
);

#elif __arm__

__asm__ (
".section .entry;"
".global entry;"
"entry:"
"    nop;"
"    bl main;"
"    .inst 0xe7f001f0"
);

#elif __aarch64__

__asm__ (
".section .entry;"
".global entry;"
"entry:"
"    nop;"
"    bl main;"
"    .inst 0xd4200000"
);

#else
#error "unknown arch"
#endif
