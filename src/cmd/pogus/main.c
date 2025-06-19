#include "core/include.h"

#include "engine/include.h"

static void
run_main_loop(EngineHarness* h) {
    if (h->exit) {
        return;
    }

    log_debug(&h->lg, ss("main loop enter"));

    uint frame = 0;
    while (!h->exit) {        
        poll_system_events(h);
        if (h->exit) {
            log_debug(&h->lg, ss("main loop exit"));
            return;
        }

        frame += 1;
    }
}

static void
log_exit_code(Logger* lg, uint code) {
    str msg = ss("program exit");
    LogField field = log_field_u64(ss("code"), code);

    if (code == 0) {
        log_info_field(lg, msg, field);
    } else {
        log_warn_field(lg, msg, field);
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

    log_exit_code(&h.lg, h.exit_code);
    log_sink_close(&h.log_sink);

    return h.exit_code;
}
