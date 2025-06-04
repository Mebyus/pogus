#define LOG_LEVEL_FATAL 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_INFO  3
#define LOG_LEVEL_DEBUG 4

#define LOG_BUFFER_SIZE (1 << 14)

typedef struct {
    u8 buf[LOG_BUFFER_SIZE];

    // Write position for buffer. Corresponds to number of bytes
    // currently stored in buffer.
    uint pos;

    uint fd;

    u8 level;
} Logger;

static void
init_log(Logger* lg, str path, u8 level) {
    u8 path_buf[OS_LINUX_MAX_PATH_LENGTH];
    c_string cstr_path = unsafe_copy_as_c_string(make_span_u8(path_buf, OS_LINUX_MAX_PATH_LENGTH), path);

    u32 flags = OS_LINUX_OPEN_FLAG_CREATE | OS_LINUX_OPEN_FLAG_TRUNCATE | OS_LINUX_OPEN_FLAG_WRITE_ONLY;
    sint n = os_linux_amd64_syscall_open(cstr_path.ptr, flags, 0644);
    if (n > 0) {
        lg->fd = cast(uint, n);
    } else {
        lg->fd = 0;
    }
    
    lg->pos = 0;
    lg->level = level;
}

static bool
log_buffer_full(Logger *lg) {
    return lg->pos >= LOG_BUFFER_SIZE;
}

static span_u8
log_buffer_tail(Logger* lg) {
    return make_span_u8(lg->buf + lg->pos, LOG_BUFFER_SIZE - lg->pos);
}

static span_u8
log_buffer_head(Logger* lg) {
    return make_span_u8(lg->buf, lg->pos);
}

static void
log_flush(Logger* lg) {
    if (lg->fd != 0) {
        os_linux_write_all(lg->fd, log_buffer_head(lg));
    }
    lg->pos = 0;
}

static void
log_put_byte(Logger* lg, u8 b) {
    if (log_buffer_full(lg)) {
        log_flush(lg);
    }

    span_u8 tail = log_buffer_tail(lg);
    tail.ptr[0] = b;
    lg->pos += 1;
}

static void
log_newline(Logger* lg) {
    log_put_byte(lg, '\n');
}

static void
log_write(Logger* lg, str s) {
    if (s.len >= LOG_BUFFER_SIZE) {
        // avoid copies for large strings
        log_flush(lg);
        if (lg->fd != 0) {
            os_linux_write_all(lg->fd, s);
        }
        return;
    }

    uint i = 0; // number of bytes from s written
    while (i < s.len) {
        if (log_buffer_full(lg)) {
            log_flush(lg);
        }

        span_u8 tail = log_buffer_tail(lg);
        uint n = copy(tail, str_slice_tail(s, i));
        i += n;
        lg->pos += n;
    }
}

static void
log_message(Logger* lg, u8 level, str s) {
    if (level > lg->level) {
        return;
    }

    // TODO: add prefix
    log_write(lg, s);
    log_newline(lg);
}

static void
log_debug(Logger* lg, str s) {
    log_message(lg, LOG_LEVEL_DEBUG, s);
}

static void
log_info(Logger* lg, str s) {
    log_message(lg, LOG_LEVEL_INFO, s);
}

static void
log_warn(Logger* lg, str s) {
    log_message(lg, LOG_LEVEL_WARN, s);
}

static void
log_error(Logger* lg, str s) {
    log_message(lg, LOG_LEVEL_ERROR, s);
}
