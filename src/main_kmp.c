#include "core/include.h"

uint main(uint argc, u8** argv, u8** envp) {
    init_proc_mem_bump_allocator();

    str s = ss("AAAAAAAAAAAAAABAAAAAAAAAAAAAAB");
    str p = ss("AAB");

    RetIndex r = str_index(s, p);
    if (!r.ok) {
        print(ss("not found\n"));
        // return 0;
    }

    u8 array_buf[1 << 14];
    FormatBuffer buf;
    init_fmt_buffer(&buf, make_span_u8(array_buf, array_len(array_buf)));

    unsafe_fmt_buffer_put_dec_u64(&buf, r.index);
    unsafe_fmt_buffer_put_newline(&buf);
    print(fmt_buffer_head(&buf));


    uint count = str_count_prefix_repeats(s, p);

    fmt_buffer_reset(&buf);
    unsafe_fmt_buffer_put_str(&buf, ss("prefix repeats: "));
    unsafe_fmt_buffer_put_dec_u64(&buf, count);
    unsafe_fmt_buffer_put_newline(&buf);
    print(fmt_buffer_head(&buf));

    RetPrefixRepeat ret = str_find_optimal_prefix_repeats(s);

    fmt_buffer_reset(&buf);
    unsafe_fmt_buffer_put_str(&buf, ss("best prefix: "));
    unsafe_fmt_buffer_put_str(&buf, ret.prefix);
    unsafe_fmt_buffer_put_newline(&buf);
    unsafe_fmt_buffer_put_str(&buf, ss("best count: "));
    unsafe_fmt_buffer_put_dec_u64(&buf, ret.count);
    unsafe_fmt_buffer_put_newline(&buf);
    unsafe_fmt_buffer_put_str(&buf, ss("best total length: "));
    unsafe_fmt_buffer_put_dec_u64(&buf, ret.count * ret.prefix.len);
    unsafe_fmt_buffer_put_newline(&buf);
    unsafe_fmt_buffer_put_str(&buf, ss("best weight: "));
    unsafe_fmt_buffer_put_dec_u64(&buf, ret.weight);
    unsafe_fmt_buffer_put_newline(&buf);
    print(fmt_buffer_head(&buf));
    // uint lps_buf[1 << 10];
    // span_uint lps = make_span_uint(lps_buf, p.len);
    // unsafe_str_fill_longest_prefix_suffix(lps, p);
    
    // fmt_buffer_reset(&buf);
    // unsafe_fmt_buffer_put_dec_span_uint(&buf, lps);
    // unsafe_fmt_buffer_put_newline(&buf);
    // print(fmt_buffer_head(&buf));

    return 0;
}
