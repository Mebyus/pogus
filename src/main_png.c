#include "core/include.h"

#include "crc.c"
#include "png/include.h"

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

    init_crc_table();

    str path = os_proc_input.args.ptr[1];
    MemBlob blob;
    code = os_load_file(imake_mem_bump_allocator(&proc_mem_bump_allocator), path, &blob);
    if (code != 0) {
        print(ss("failed to read file\n"));
        return code;
    }

    png_print(mem_blob_get_data(blob));

    return 0;
}
