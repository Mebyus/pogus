#include "types.c"
#include "os_linux_amd64.c"
#include "link_libc.h"
#include "window_linux.c"

static void
run_main_loop(EngineHarness* h) {
    while (!h->exit) {
        poll_system_events(h);
        if (h->exit) {
            return;
        }
    }
}

uint main(uint argc, u8** argv, u8** envp) {
    init_proc_mem_bump_allocator();
    ErrorCode code = init_os_proc_input(argc, argv, envp);
    if (code != 0) {
        return code;
    }

    EngineHarness h = {};
    h.title = ss("Pogus");

    init_engine_harness(&h);
    run_main_loop(&h);

    return h.exit_code;
}
