#include "types.c"
#include "os_linux_amd64.c"

#define BUFFER_SIZE 1024

uint main(uint argc, u8** argv, u8** envp) {
    init_proc_mem_bump_allocator();
    ErrorCode code = init_os_proc_input(argc, argv, envp);
    if (code != 0) {
        return code;
    }

    for (uint i = 0; i < os_proc_input.args.len; i += 1) {
        print(os_proc_input.args.ptr[i]);
        print(ss("\n"));
    }

    print(ss("hello, world!\n"));

    MemBlock block;
    block.span.len = BUFFER_SIZE + 1;
    ErrorCode c = proc_mem_alloc(&block);
    if (c != 0) {
        return 4;
    }

    MemBlob blob;
    c = os_load_file(imake_mem_bump_allocator(&proc_mem_bump_allocator), ss("build.claw"), &blob);

    FormatBuffer buf;
    init_fmt_buffer(&buf, block.span);
    unsafe_fmt_buffer_put_dec_u64(&buf, blob.block.id);
    unsafe_fmt_buffer_put_newline(&buf);

    span_u8 data = mem_blob_get_data(blob);
    unsafe_fmt_buffer_put_hex_prefix_zeroes_u64(&buf, djb2_hash64(data));
    unsafe_fmt_buffer_put_newline(&buf);

    print(fmt_buffer_head(&buf));
    print(mem_blob_get_data(blob));
    return 0;
}
