#include "core/include.h"

uint main(uint argc, u8** argv, u8** envp) {
    init_proc_mem_bump_allocator();
    ErrorCode code = init_os_proc_input(argc, argv, envp);
    if (code != 0) {
        return code;
    }

    if (os_proc_input.args.len < 3) {
        print(ss("files not specified\n"));
        return 2;
    }

    str source_path = os_proc_input.args.ptr[1];
    str target_path = os_proc_input.args.ptr[2];

    RetOpen source_ret = os_open(source_path);
    if (source_ret.code != 0) {
        print(ss("unable to open source file\n"));
        return 3;
    }

    RetOpen target_ret = os_create(target_path);
    if (target_ret.code != 0) {
        print(ss("unable to create target file\n"));
        return 3;
    }

    Reader r = bag_fd_reader(source_ret.fd);
    Writer w = bag_fd_writer(target_ret.fd);
    RetCopy ret = bag_copy(w, r);
    if (ret.code != 0) {
        print(ss("error while copying the file\n"));
        return 4;
    }
    return 0;
}
