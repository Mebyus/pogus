#include "core/include.h"

uint main(uint argc, u8** argv, u8** envp) {
    init_proc_mem_bump_allocator();
    ErrorCode code = init_os_proc_input(argc, argv, envp);
    if (code != 0) {
        return code;
    }

    RetOpen target_ret = os_create(ss("lines_out.log"));
    if (target_ret.code != 0) {
        print(ss("unable to create target file\n"));
        return 3;
    }

    str lines_array[] = {
        sl("[fatal] "),
        sl("[error] "),
        sl(" [warn] "),
        sl(" [info] "),
        sl("[debug] "),
    };
    span_str lines = make_span_str(lines_array, array_len(lines_array));

    LinesReader lr;
    init_lines_reader(&lr, lines);

    Reader r = bag_lines_reader(&lr);
    Writer w = bag_fd_writer(target_ret.fd);
    RetCopy ret = bag_copy(w, r);
    if (ret.code != 0) {
        print(ss("error while copying the file\n"));
        return 4;
    }
    return 0;
}
