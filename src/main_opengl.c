#include "types.c"
#include "os_linux_amd64.c"
#include "os_linux_log.c"
#include "rand.c"
#include "sort.c"
#include "strconv.c"

uint main(uint argc, u8** argv, u8** envp) {
    init_proc_mem_bump_allocator();
    ErrorCode code = init_os_proc_input(argc, argv, envp);
    if (code != 0) {
        return code;
    }
    
    Logger lg;
    init_log(&lg, ss("opengl.log"), LOG_LEVEL_DEBUG);

    // number of generated integers
    uint num_gen = 1 << 27;
    if (os_proc_input.args.len >= 2) {
        str num_str = os_proc_input.args.ptr[1];
        RetParseU64 r = parse_dec_u64(num_str);
        if (r.code != 0) {
            return r.code;
        }
        num_gen = r.n;
    }

    MemBlock block = {};
    block.span.len = num_gen * sizeof(s64);
    code = os_linux_mem_alloc(&block);
    
    if (code != 0) {
        log_error_field(&lg, ss("allocate memory from os"), log_field_u64(ss("code"), code));
    } else {
        log_debug_field(&lg, ss("allocate memory from os"), log_field_ptr(ss("ptr"), block.span.ptr));
    }

    // os_linux_mem_free(block);

    Biski64State state;
    biski64_seed(&state, 123);
    
    // s64 array[] = {3, 4, 0, -2, 0, 1, -5, 10, 11, 12, 0, 12, 4, 3, 1, 1};
    span_s64 s = make_span_s64(cast(s64*, block.span.ptr), num_gen);
    log_debug_field(&lg, ss("generate test data"), log_field_u64(ss("len"), num_gen));
    biski64_fill_s64(&state, s);
    // log_debug_field(&lg, ss("test before sort"), log_field_span_s64(ss("span"), s));
    quick_sort_s64(s);

    if (is_sorted_asc_s64(s)) {
        log_info(&lg, ss("sorted successfully"));
    } else {
        log_error(&lg, ss("not sorted"));
    }

    // log_debug_field(&lg, ss("test span output"), log_field_span_s64(ss("span"), s));
    // log_debug_field(&lg, ss("test array size"), log_field_u64(ss("size"), sizeof(array)));
    // log_debug_field(&lg, ss("test array len"), log_field_u64(ss("len"), array_len(array)));
    
    log_close(&lg);
    return 0;
}
