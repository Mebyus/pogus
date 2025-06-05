#include "types.c"
#include "os_linux_amd64.c"
#include "link_libc.h"
#include "os_linux_log.c"
#include "window_linux.c"
#include "graphics_vulkan.c"

static void
run_main_loop(EngineHarness* h) {
    if (h->exit) {
        return;
    }

    log_info(&h->lg, ss("main loop enter"));

    uint frame = 0;
    while (!h->exit) {        
        poll_system_events(h);
        if (h->exit) {
            log_info(&h->lg, ss("main loop exit"));
            return;
        }

        frame += 1;
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
    log_flush(&h.lg);

    return h.exit_code;
}
