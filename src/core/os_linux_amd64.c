#define OS_LINUX_ERROR_CODE_NOT_EXIST 2

#define OS_LINUX_AMD64_SYSCALL_READ 0

static sint
os_linux_amd64_syscall_read(uint fd, void* buf, uint size)
{
    sint ret;
    __asm__ volatile
    (
        "syscall"
		
        // outputs
		// RAX
        : "=a" (ret)

        // inputs
		// RAX
        : "0"(OS_LINUX_AMD64_SYSCALL_READ), 
        //  RDI      RSI       RDX
			"D"(fd), "S"(buf), "d"(size)

		// two registers are clobbered after system call
        : "rcx", "r11", 
			"memory"
    );
    return ret;
}

#define OS_LINUX_AMD64_SYSCALL_WRITE 1

static sint
os_linux_amd64_syscall_write(uint fd, const void* buf, uint size)
{
    sint ret;
    __asm__ volatile
    (
        "syscall"
		
        // outputs
		// RAX
        : "=a" (ret)

        // inputs
		// RAX
        : "0"(OS_LINUX_AMD64_SYSCALL_WRITE), 
        //  RDI      RSI       RDX
			"D"(fd), "S"(buf), "d"(size)

		// two registers are clobbered after system call
        : "rcx", "r11", 
			"memory"
    );
    return ret;
}


#define OS_LINUX_AMD64_SYSCALL_OPEN 2

static sint
os_linux_amd64_syscall_open(const u8* path, u32 flags, u32 mode)
{
    sint ret;
    __asm__ volatile
    (
        "syscall"
		
        // outputs
		// RAX
        : "=a" (ret)

        // inputs
		// RAX
        : "0"(OS_LINUX_AMD64_SYSCALL_OPEN), 
        //  RDI      RSI       RDX
			"D"(path), "S"(flags), "d"(mode)

		// two registers are clobbered after system call
        : "rcx", "r11", 
			"memory"
    );
    return ret;
}

static ErrorCode
os_linux_convert_syscall_open_error(uint c) {
    return c; // TODO: make proper error conversion
}

#define OS_LINUX_OPEN_FLAG_READ_ONLY  0x0
#define OS_LINUX_OPEN_FLAG_WRITE_ONLY 0x1
#define OS_LINUX_OPEN_FLAG_READ_WRITE 0x2

#define OS_LINUX_OPEN_FLAG_CREATE   0x40
#define OS_LINUX_OPEN_FLAG_TRUNCATE 0x200
#define OS_LINUX_OPEN_FLAG_APPEND   0x400

#define OS_LINUX_AMD64_SYSCALL_CLOSE 3

static sint
os_linux_amd64_syscall_close(uint fd)
{
    sint ret;
    __asm__ volatile
    (
        "syscall"
		
        // outputs
		// RAX
        : "=a" (ret)

        // inputs
		// RAX
        : "0"(OS_LINUX_AMD64_SYSCALL_CLOSE), 
        //  RDI
			"D"(fd)

		// two registers are clobbered after system call
        : "rcx", "r11", 
			"memory"
    );
    return ret;
}

typedef struct {
  s64 sec;
  s64 nano;
} LinuxTimespec;

typedef struct {
    // ID of device containing file
    u64 device;

    // Inode number
    u64 inode;

    // Number of hard links
    u64 num_links;

    // File type and mode
    u32 mode;

    // User ID of owner
    u32 user_id;

    // Group ID of owner
    u32 group_id;

    // This field is placed here purely for padding. It does not
    // carry any useful information
    u32 padding;

    // Device ID (if special file)
    u64 r_device;

    // Total size in bytes
    u64 size;

    // Block size for filesystem I/O
    u64 block_size;

    // Number of 512B blocks allocated
    u64 num_blocks;

    // Time of last access
    LinuxTimespec access_time;

    // Time of last modification
    LinuxTimespec mod_time;

    // Time of last status change
    LinuxTimespec status_change_time;

    // Additional reserved fields for future compatibility
    u64 reserved[3];
} LinuxFileStat;

#define OS_LINUX_AMD64_SYSCALL_STAT 4

static sint
os_linux_amd64_syscall_stat(const u8* path, LinuxFileStat* stat)
{
    sint ret;
    __asm__ volatile
    (
        "syscall"
		
        // outputs
		// RAX
        : "=a" (ret)

        // inputs
		// RAX
        : "0"(OS_LINUX_AMD64_SYSCALL_STAT), 
        //  RDI      RSI
			"D"(path), "S"(stat)

		// two registers are clobbered after system call
        : "rcx", "r11", 
			"memory"
    );
    return ret;
}

static ErrorCode
os_linux_convert_syscall_stat_error(uint c) {
    return c; // TODO: make proper error conversion
}

#define OS_LINUX_AMD64_SYSCALL_MMAP 9

#define OS_LINUX_MEMORY_MAP_PROT_READ  0x1
#define OS_LINUX_MEMORY_MAP_PROT_WRITE 0x2

#define OS_LINUX_MEMORY_MAP_SHARED    0x01
#define OS_LINUX_MEMORY_MAP_PRIVATE   0x02
#define OS_LINUX_MEMORY_MAP_ANONYMOUS 0x20

static sint
os_linux_amd64_syscall_mmap(void* ptr, uint len, uint prot, uint flags, uint fd, uint offset) {
    register sint  rax __asm__ ("rax") = OS_LINUX_AMD64_SYSCALL_MMAP;
    register void* rdi __asm__ ("rdi") = ptr;
    register uint  rsi __asm__ ("rsi") = len;
    register uint  rdx __asm__ ("rdx") = prot;
    register uint  r10 __asm__ ("r10") = flags;
    register uint  r8  __asm__ ("r8")  = fd;
    register uint  r9  __asm__ ("r9")  = offset;
    __asm__ __volatile__ (
        "syscall"
        : "+r" (rax)
        : "r" (rdi), "r" (rsi), "r" (rdx), "r" (r10), "r" (r8), "r" (r9)
        : "rcx", "r11", "memory"
    );
    return rax;
}

#define OS_LINUX_AMD64_SYSCALL_MUNMAP 11

static sint
os_linux_amd64_syscall_munmap(void* ptr, uint len) {
    register sint  rax __asm__ ("rax") = OS_LINUX_AMD64_SYSCALL_MUNMAP;
    register void* rdi __asm__ ("rdi") = ptr;
    register uint  rsi __asm__ ("rsi") = len;
    __asm__ __volatile__ (
        "syscall"
        : "+r" (rax)
        : "r" (rdi), "r" (rsi)
        : "rcx", "r11", "memory"
    );
    return rax;
}

#define OS_LINUX_AMD64_SYSCALL_EXIT 60

static _Noreturn void
os_linux_amd64_syscall_exit(uint c) {
    sint ret;
    __asm__ volatile
    (
        "syscall"
		
		// outputs
		// RAX
        : "=a" (ret)
		
		// inputs
		// RAX
        : "0"(OS_LINUX_AMD64_SYSCALL_EXIT), 
        //  RDI     
			"D"(c)

		// clobbers
		// two registers are clobbered after system call
        : "rcx", "r11"
    );
	__builtin_unreachable();
}

static _Noreturn void
os_exit(uint c) {
    os_linux_amd64_syscall_exit(c);
}

static ErrorCode
os_linux_convert_syscall_write_error(uint c) {
    return c; // TODO: make proper error conversion
}

static RetWrite
os_linux_write(uint fd, span_u8 buf) {
    RetWrite ret = {};
    if (buf.len == 0) {
        return ret;
    }
    must(buf.ptr != nil);

    sint n = os_linux_amd64_syscall_write(fd, buf.ptr, buf.len);
    if (n < 0) {
        ret.code = os_linux_convert_syscall_write_error(cast(uint, -n));
        return ret;
    }

    ret.count = cast(uint, n);
    return ret;
}

#define OS_LINUX_STDOUT 1
#define OS_LINUX_STDERR 2

static RetWrite
os_linux_write_all(uint fd, str s) {
    RetWrite ret = {};

	if (s.len == 0) {
		return ret;
	}

	while (ret.count < s.len) {
		RetWrite r = os_linux_write(fd, span_u8_slice_tail(s, ret.count));
        ret.count += r.count;
		
        if (r.code != 0) {
            ret.code = r.code;
			return ret;
		}
	}

    return ret;
}

static void stdout_write(span_u8 s) {
    os_linux_write_all(OS_LINUX_STDOUT, s);
}

static void stderr_write(span_u8 s) {
    os_linux_write_all(OS_LINUX_STDERR, s);
}

static ErrorCode
os_linux_convert_syscall_read_error(uint c) {
    return c; // TODO: make proper error conversion
}

static RetRead
os_linux_read(uint fd, span_u8 buf) {
    RetRead ret = {};
    if (buf.len == 0) {
        return ret;
    }
    must(buf.ptr != nil);

    sint n = os_linux_amd64_syscall_read(fd, buf.ptr, buf.len);
    if (n == 0) {
        ret.code = ERROR_READER_EOF;
        return ret;
    }
    if (n < 0) {
        ret.code = os_linux_convert_syscall_read_error(cast(uint, -n));
        return ret;
    }

    ret.count = cast(uint, n);
    return ret;
}

/*
Reads until reaching EOF on given file descriptor or no more
space left in buffer.
*/
static RetRead
os_linux_read_all(uint fd, span_u8 buf) {
    RetRead ret = {};

	if (buf.len == 0) {
		return ret;
	}

    while (ret.count < buf.len) {
        RetRead r = os_linux_read(fd, buf);
        ret.count += r.count;
        
        if (r.code != 0) {
            ret.code = r.code;
            return ret;
        }
    }

    return ret;
}


static ErrorCode
os_linux_convert_syscall_mmap_error(uint c) {
    return c; // TODO: make proper error conversion
}

#define OS_LINUX_PAGE_SIZE (1 << 12)

/*/doc

Allocates memory in page-sized chunks directly from operating system.
*/
static ErrorCode
os_linux_mem_alloc(MemBlock* block) {
    uint len = block->span.len;
    must(len != 0);

    len = align_uint(len, OS_LINUX_PAGE_SIZE);

    const uint prot = OS_LINUX_MEMORY_MAP_PROT_READ | OS_LINUX_MEMORY_MAP_PROT_WRITE;
    const uint flags = OS_LINUX_MEMORY_MAP_PRIVATE | OS_LINUX_MEMORY_MAP_ANONYMOUS;
    sint n = os_linux_amd64_syscall_mmap(nil, len, prot, flags, 0, 0);
    if (n < 0) {
        return os_linux_convert_syscall_mmap_error(cast(uint, -n));
    }

    block->span.ptr = cast(u8*, n);
    block->span.len = len;
    block->id = 0;
    return 0;
}

static void
os_linux_mem_free(MemBlock block) {
    sint n = os_linux_amd64_syscall_munmap(block.span.ptr, block.span.len);
    if (n < 0) {
        panic_trap();
    }
}

/*
Represents a blob that was fully loaded into memory.
*/
typedef struct {
    // Memory block that holds loaded data.
    MemBlock block;

    // Actual blob size in bytes.
    uint size;
} MemBlob;

static span_u8
mem_blob_get_data(MemBlob blob) {
    return make_span_u8(blob.block.span.ptr, blob.size);
}

static void
clear_mem_blob(MemBlob* blob) {
    clear_mem_block(&blob->block);
	blob->size = 0;
}

#define OS_LINUX_MAX_PATH_LENGTH (1 << 14)

#define ERROR_LONG_PATH 2

typedef struct {
    // File descriptor of opened file.
    uint fd;

    ErrorCode code;
} RetOpen;

static RetOpen
os_open_file(str path, u32 flags, u32 mode) {
    must(path.len != 0);

    RetOpen ret = {};
    if (path.len >= OS_LINUX_MAX_PATH_LENGTH) {
        ret.code = ERROR_LONG_PATH;
        return ret;
    }

    u8 path_buf[OS_LINUX_MAX_PATH_LENGTH];
    c_string cstr_path = unsafe_copy_as_c_string(make_span_u8(path_buf, OS_LINUX_MAX_PATH_LENGTH), path);

    sint n = os_linux_amd64_syscall_open(cstr_path.ptr, flags, mode);
    if (n < 0) {
        ret.code = os_linux_convert_syscall_open_error(cast(uint, -n)); 
        return ret;
    }
    ret.fd = cast(uint, n);
    return ret;
}

static RetOpen
os_open(str path) {
    return os_open_file(path, OS_LINUX_OPEN_FLAG_READ_ONLY, 0);
}

static RetOpen
os_create(str path) {
    return os_open_file(path, OS_LINUX_OPEN_FLAG_WRITE_ONLY | OS_LINUX_OPEN_FLAG_CREATE | OS_LINUX_OPEN_FLAG_TRUNCATE, 0644);
}

/*
Reads entire file into memory provided by allocator.

File must exist. Must be regular file.
*/
static ErrorCode
os_load_file(MemAllocator al, str path, MemBlob* blob) {
    must(path.len != 0);

    if (path.len >= OS_LINUX_MAX_PATH_LENGTH) {
        return ERROR_LONG_PATH;
    }

    u8 path_buf[OS_LINUX_MAX_PATH_LENGTH];
    c_string cstr_path = unsafe_copy_as_c_string(make_span_u8(path_buf, OS_LINUX_MAX_PATH_LENGTH), path);

    LinuxFileStat stat;
    sint n = os_linux_amd64_syscall_stat(cstr_path.ptr, &stat);
    if (n < 0) {
        return os_linux_convert_syscall_stat_error(cast(uint, -n));
    }
    if (stat.size == 0) {
        clear_mem_blob(blob);
        return 0;
    }

    n = os_linux_amd64_syscall_open(cstr_path.ptr, OS_LINUX_OPEN_FLAG_READ_ONLY, 0);
    if (n < 0) {
        return os_linux_convert_syscall_open_error(cast(uint, -n));
    }
    uint fd = cast(uint, n);

    blob->block.span.len = stat.size;
    blob->size = stat.size;
    ErrorCode code = mem_alloc(al, &blob->block);
    if (code != 0) {
        os_linux_amd64_syscall_close(fd);
        return code;
    }

    RetRead r = os_linux_read_all(fd, blob->block.span);
    os_linux_amd64_syscall_close(fd);
    if (r.code != 0 && r.code != ERROR_READER_EOF) {
        mem_free(al, blob->block);
        return r.code;
    }

    return 0;
}

const BagReaderTab bag_fd_reader_tab = {
    .type_id = 3,
    .read = cast(BagFuncRead, cast(void*, os_linux_read)),
};

static Reader
bag_fd_reader(uint fd) {
    Reader r = {};
    r.ptr = cast(void*, fd); // TODO: maybe we should change void* in bag to just uint and name it "val"
    r.tab = &bag_fd_reader_tab;
    return r;    
}

const BagWriterTab bag_fd_writer_tab = {
    .type_id = 4,
    .write = cast(BagFuncWrite, cast(void*, os_linux_write)),
};

static Writer
bag_fd_writer(uint fd) {
    Writer w = {};
    w.ptr = cast(void*, fd);
    w.tab = &bag_fd_writer_tab;
    return w;    
}
