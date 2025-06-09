#include "core/include.h"

#include "crc.c"

uint main(uint argc, u8** argv, u8** envp) {
    init_proc_mem_bump_allocator();
    ErrorCode code = init_os_proc_input(argc, argv, envp);
    if (code != 0) {
        return code;
    }

    if (os_proc_input.args.len < 2) {
        print(ss("file not specified\n"));
        return 2;
    }

    str path = os_proc_input.args.ptr[1];
    RetOpen o = os_open(path);
    if (o.code != 0) {
        return o.code;
    }

    init_crc_table();
    u32 crc = 0;
    u8 buf_array[1 << 14];
    span_u8 buf = make_span_u8(buf_array, array_len(buf_array));
    while (true) {
        RetRead r = os_linux_read(o.fd, buf);
        crc = crc_digest(crc, span_u8_slice_head(buf, r.count));
        if (r.code != 0) {
            if (r.code == ERROR_READER_EOF) {
                break;
            }
            return r.code;
        }

    }

    FormatBuffer f;
    init_fmt_buffer(&f, buf);
    unsafe_fmt_buffer_put_hex_prefix_zeroes_u32(&f, crc);
    unsafe_fmt_buffer_put_newline(&f);
    print(fmt_buffer_head(&f));
    return 0;
}
