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

#define LOG_FIELD_U64 0
#define LOG_FIELD_S64 1
#define LOG_FIELD_STR 2
#define LOG_FIELD_PTR 3

typedef union {
    u64 u64;
    s64 s64;
    str str;

    void* ptr;
} LogFieldValue;

typedef struct {
    LogFieldValue value;

    str name;
    u8  kind;
} LogField;

static LogField
log_field_u64(str name, u64 value) {
    LogField field = {};
    field.value.u64 = value;
    field.name = name;
    field.kind = LOG_FIELD_U64;
    return field;
}

static LogField
log_field_s64(str name, s64 value) {
    LogField field = {};
    field.value.s64 = value;
    field.name = name;
    field.kind = LOG_FIELD_S64;
    return field;
}

static LogField
log_field_str(str name, str value) {
    LogField field = {};
    field.value.str = value;
    field.name = name;
    field.kind = LOG_FIELD_STR;
    return field;
}

static LogField
log_field_ptr(str name, void* value) {
    LogField field = {};
    field.value.ptr = value;
    field.name = name;
    field.kind = LOG_FIELD_PTR;
    return field;    
}

static str
log_prefix_table[LOG_LEVEL_DEBUG + 1];

static void
init_log(Logger* lg, str path, u8 level) {
    static_assert(LOG_BUFFER_SIZE >= 1024);
    must(path.len != 0);

    // TODO: make prefix initialization once per program start
    log_prefix_table[LOG_LEVEL_FATAL] = ss("[fatal] ");
    log_prefix_table[LOG_LEVEL_ERROR] = ss("[error] ");
    log_prefix_table[LOG_LEVEL_WARN]  = ss(" [warn] ");
    log_prefix_table[LOG_LEVEL_INFO]  = ss(" [info] ");
    log_prefix_table[LOG_LEVEL_DEBUG] = ss("[debug] ");

    lg->fd = 0;
    lg->pos = 0;
    lg->level = level;

    if (path.len >= OS_LINUX_MAX_PATH_LENGTH) {
        return;
    }

    u8 path_buf[OS_LINUX_MAX_PATH_LENGTH];
    c_string cstr_path = unsafe_copy_as_c_string(make_span_u8(path_buf, OS_LINUX_MAX_PATH_LENGTH), path);

    u32 flags = OS_LINUX_OPEN_FLAG_CREATE | OS_LINUX_OPEN_FLAG_TRUNCATE | OS_LINUX_OPEN_FLAG_WRITE_ONLY;
    sint n = os_linux_amd64_syscall_open(cstr_path.ptr, flags, 0644);
    if (n <= 0) {
        return;
    }

    lg->fd = cast(uint, n);
}

static bool
log_buffer_full(Logger *lg) {
    return lg->pos >= LOG_BUFFER_SIZE;
}

/*/doc

Returns length (in bytes) of buffer tail (unoccupied portion).
*/
static uint
log_buffer_left(Logger *lg) {
    return LOG_BUFFER_SIZE - lg->pos;
}

static span_u8
log_buffer_tail(Logger* lg) {
    return make_span_u8(lg->buf + lg->pos, log_buffer_left(lg));
}

static span_u8
log_buffer_head(Logger* lg) {
    return make_span_u8(lg->buf, lg->pos);
}

static void
log_file_write(Logger* lg, span_u8 s) {
    if (lg->fd == 0) {
        return;
    }

    os_linux_write_all(lg->fd, s);
}

static void
log_file_close(Logger* lg) {
    if (lg->fd == 0) {
        return;
    }
    
    os_linux_amd64_syscall_close(lg->fd);
}

static void
log_flush(Logger* lg) {
    log_file_write(lg, log_buffer_head(lg));
    lg->pos = 0;
}

static void
log_close(Logger* lg) {
    log_flush(lg);
    log_file_close(lg);
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
log_space(Logger* lg) {
    log_put_byte(lg, ' ');
}

static void
log_write(Logger* lg, str s) {
    if (s.len >= (LOG_BUFFER_SIZE / 2)) {
        // avoid copies for large strings
        log_flush(lg);
        log_file_write(lg, s);
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
log_write_field_value_u64(Logger* lg, u64 value) {
    if (log_buffer_left(lg) < max_u64_dec_length) {
        log_flush(lg);
    }

    span_u8 tail = log_buffer_tail(lg);
    uint n = unsafe_fmt_dec_u64(tail, value);
    lg->pos += n;
}

static void
log_write_field_value_s64(Logger* lg, s64 value) {
    if (log_buffer_left(lg) < max_s64_dec_length) {
        log_flush(lg);
    }

    span_u8 tail = log_buffer_tail(lg);
    uint n = unsafe_fmt_dec_s64(tail, value);
    lg->pos += n;
}

static void
log_write_field_value_str(Logger* lg, str value) {
    log_put_byte(lg, '"');
    log_write(lg, value);
    log_put_byte(lg, '"');
}

static void
log_write_field_value_ptr(Logger* lg, void* ptr) {
    const uint fmt_ptr_length = 2 + 16; // 0x + hex number
    if (log_buffer_left(lg) < fmt_ptr_length) {
        log_flush(lg);
    }

    span_u8 tail = log_buffer_tail(lg);
    tail.ptr[0] = '0';
    tail.ptr[1] = 'x';
    unsafe_fmt_hex_prefix_zeroes_u64(span_u8_slice_tail(tail, 2), cast(u64, ptr));
    lg->pos += fmt_ptr_length;
}

static void
log_write_field_value(Logger* lg, u8 kind, LogFieldValue value) {
    switch (kind) {
    case LOG_FIELD_U64:
        log_write_field_value_u64(lg, value.u64);
        return;
    case LOG_FIELD_S64:
        log_write_field_value_s64(lg, value.s64);
        return;
    case LOG_FIELD_STR:
        log_write_field_value_str(lg, value.str);
        return;
    case LOG_FIELD_PTR:
        log_write_field_value_ptr(lg, value.ptr);
        return;
    default:
        panic_trap();
    }
}

static void
log_write_field(Logger* lg, LogField field) {
    log_put_byte(lg, '{');
    log_write(lg, field.name);
    log_put_byte(lg, ':');
    log_space(lg);
    log_write_field_value(lg, field.kind, field.value);
    log_put_byte(lg, '}');
}

static void
log_message(Logger* lg, u8 level, str s) {
    if (level > lg->level) {
        return;
    }

    log_write(lg, log_prefix_table[level]);
    log_write(lg, s);
    log_newline(lg);
}

static void
log_message_field(Logger* lg, u8 level, str s, LogField field) {
    if (level > lg->level) {
        return;
    }

    log_write(lg, log_prefix_table[level]);
    log_write(lg, s);
    log_space(lg);
    log_write_field(lg, field);
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

static void
log_debug_field(Logger* lg, str s, LogField field) {
    log_message_field(lg, LOG_LEVEL_DEBUG, s, field);
}

static void
log_info_field(Logger* lg, str s, LogField field) {
    log_message_field(lg, LOG_LEVEL_INFO, s, field);
}

static void
log_warn_field(Logger* lg, str s, LogField field) {
    log_message_field(lg, LOG_LEVEL_WARN, s, field);
}

static void
log_error_field(Logger* lg, str s, LogField field) {
    log_message_field(lg, LOG_LEVEL_ERROR, s, field);
}
