#include "types.c"
#include "os_linux_amd64.c"
#include "os_linux_log.c"
#include "rand.c"

uint main(uint argc, u8** argv, u8** envp) {
    init_proc_mem_bump_allocator();
    ErrorCode code = init_os_proc_input(argc, argv, envp);
    if (code != 0) {
        return code;
    }

    Logger lg;
    init_log(&lg, ss("opengl.log"), LOG_LEVEL_DEBUG);

    MemBlock block = {};
    block.span.len = 34;
    code = os_linux_mem_alloc(&block);
    
    if (code != 0) {
        log_error_field(&lg, ss("allocate memory from os"), log_field_u64(ss("code"), code));
    } else {
        log_debug_field(&lg, ss("allocate memory from os"), log_field_ptr(ss("ptr"), block.span.ptr));
    }

    os_linux_mem_free(block);

    Biski64State state;
    biski64_seed(&state, 123);
    
    uint count1 = 0;
    uint count2 = 0; 
    uint count11 = 0;
    for (uint i = 0; i < (1 << 26); i += 1) {
        u64 n = biski64_next(&state);
        u64 bit1 = n & 1;
        u64 bit2 = (n >> 1) & 1;
        
        count1 += bit1;
        count2 += bit2;
        count11 += bit1 & bit2;
        // log_debug_field(&lg, ss("generate random number"), log_field_u64(ss("n"), n));
    }
    log_debug_field(&lg, ss("random number stats"), log_field_u64(ss("count1"), count1));
    log_debug_field(&lg, ss("random number stats"), log_field_u64(ss("count2"), count2));
    log_debug_field(&lg, ss("random number stats"), log_field_u64(ss("count11"), count11));

    log_close(&lg);
    return 0;
}
