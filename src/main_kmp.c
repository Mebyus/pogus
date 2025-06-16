#include "core/include.h"

uint main(uint argc, u8** argv, u8** envp) {
    init_proc_mem_bump_allocator();

    str s = ss("AABAACAADAABAABA");
    str p = ss("AABAAB");

    RetIndex r = str_index(s, p);
    if (!r.ok) {
        print(ss("not found\n"));
        return 0;
    }

    u8 array_buf[1 << 14];
    FormatBuffer buf;
    init_fmt_buffer(&buf, make_span_u8(array_buf, array_len(array_buf)));

    unsafe_fmt_buffer_put_dec_u64(&buf, r.index);
    unsafe_fmt_buffer_put_newline(&buf);
    print(fmt_buffer_head(&buf));

    return 0;
}
