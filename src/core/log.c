#define LOG_LEVEL_FATAL 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_INFO  3
#define LOG_LEVEL_DEBUG 4

#define LOG_BUFFER_SIZE (1 << 14)

/*/doc

Encapsulates buffered writes to an opened file.

Related:
    +init_log_sink(...)
    .log_sink_write(...)
    .log_sink_flush(...)
*/
typedef struct {
    u8 buf[LOG_BUFFER_SIZE];

    // Value returned by clock when sink was initialized.
    TimeDur start;

    // Write position for buffer. Corresponds to number of bytes
    // currently stored in buffer.
    uint pos;

    // File descriptor of log file. Flush is nop if this field is 0.
    uint fd;
} LogSink;

/*/doc

Init log sink with a given file descriptor. It must be open for writes
in order for logger to work properly.
*/
static void
init_log_sink_from_fd(LogSink* sink, uint fd) {
    sink->fd = fd;
    sink->pos = 0;
    sink->start = clock_mono();
}

static void
init_log_sink(LogSink* sink, str path) {
    static_assert(LOG_BUFFER_SIZE >= 1024);
    must(path.len != 0);

    RetOpen ret = os_create(path);
    if (ret.code != 0) {
        init_log_sink_from_fd(sink, 0);
        return;
    }

    init_log_sink_from_fd(sink, ret.fd);
}

static bool
log_sink_buffer_full(LogSink *sink) {
    return sink->pos >= LOG_BUFFER_SIZE;
}

/*/doc

Returns length (in bytes) of buffer tail (unoccupied portion).
*/
static uint
log_sink_buffer_left(LogSink *sink) {
    return LOG_BUFFER_SIZE - sink->pos;
}

static span_u8
log_sink_buffer_tail(LogSink* sink) {
    return make_span_u8(sink->buf + sink->pos, log_sink_buffer_left(sink));
}

static span_u8
log_sink_buffer_head(LogSink* sink) {
    return make_span_u8(sink->buf, sink->pos);
}

static void
log_sink_file_write(LogSink* sink, span_u8 s) {
    if (sink->fd == 0) {
        return;
    }

    os_linux_write_all(sink->fd, s);
}

static void
log_sink_file_close(LogSink* sink) {
    if (sink->fd == 0) {
        return;
    }
    
    os_linux_amd64_syscall_close(sink->fd);
}

static void
log_sink_flush(LogSink* sink) {
    log_sink_file_write(sink, log_sink_buffer_head(sink));
    sink->pos = 0;
}

/*/doc

Flushes log buffer if it is full. Does nothing otherwise.
*/
static void
log_sink_check_flush(LogSink* sink) {
    if (!log_sink_buffer_full(sink)) {
        return;
    }
    log_sink_flush(sink);
}

/*/doc

Flushes log buffer if less then specified number of unoccupied bytes left.
Does nothing otherwise.
*/
static void
log_sink_threshold_flush(LogSink* sink, uint threshold) {
    if (log_sink_buffer_left(sink) < threshold) {
        log_sink_flush(sink);
    }
}

static void
log_sink_close(LogSink* sink) {
    log_sink_flush(sink);
    log_sink_file_close(sink);
}

static void
log_sink_write(LogSink* sink, str s) {
    if (s.len >= (LOG_BUFFER_SIZE / 2)) {
        // avoid copies for large strings
        log_sink_flush(sink);
        log_sink_file_write(sink, s);
        return;
    }

    uint i = 0; // number of bytes from s written
    while (i < s.len) {
        log_sink_check_flush(sink);

        span_u8 tail = log_sink_buffer_tail(sink);
        uint n = copy(tail, str_slice_tail(s, i));
        i += n;
        sink->pos += n;
    }
}

static void
log_sink_put_byte(LogSink* sink, u8 b) {
    log_sink_check_flush(sink);

    span_u8 tail = log_sink_buffer_tail(sink);
    tail.ptr[0] = b;
    sink->pos += 1;
}

static void
log_sink_put_newline(LogSink* sink) {
    log_sink_put_byte(sink, '\n');
}

static void
log_sink_put_space(LogSink* sink) {
    log_sink_put_byte(sink, ' ');
}

static void
log_sink_format_logger_name(LogSink* sink, str name) {
    if (name.len == 0) {
        return;
    }

    log_sink_put_byte(sink, '(');
    log_sink_write(sink, name);
    log_sink_put_byte(sink, ')');
    log_sink_put_space(sink);
}

static void
log_sink_format_dec_u64(LogSink* sink, u64 x) {
    log_sink_threshold_flush(sink, max_u64_dec_length);

    span_u8 tail = log_sink_buffer_tail(sink);
    uint n = unsafe_fmt_dec_u64(tail, x);
    sink->pos += n;
}

static void
log_sink_format_clock(LogSink* sink) {
    TimeDur sub = time_dur_sub(clock_mono(), sink->start);

    log_sink_threshold_flush(sink, max_time_dur_micro_length);
    span_u8 tail = log_sink_buffer_tail(sink);
    uint n = unsafe_fmt_time_dur_micro(tail, sub);
    sink->pos += n;

    log_sink_put_space(sink);
}

static void
log_sink_format_dec_s64(LogSink* sink, s64 x) {
    log_sink_threshold_flush(sink, max_s64_dec_length);

    span_u8 tail = log_sink_buffer_tail(sink);
    uint n = unsafe_fmt_dec_s64(tail, x);
    sink->pos += n;
}

static void
log_sink_format_str(LogSink* sink, str s) {
    log_sink_put_byte(sink, '"');
    log_sink_write(sink, s);
    log_sink_put_byte(sink, '"');
}

static void
log_sink_format_ptr(LogSink* sink, void* ptr) {
    const uint fmt_ptr_length = 2 + 16; // 0x + hex number
    log_sink_threshold_flush(sink, fmt_ptr_length);

    span_u8 tail = log_sink_buffer_tail(sink);
    tail.ptr[0] = '0';
    tail.ptr[1] = 'x';
    unsafe_fmt_hex_prefix_zeroes_u64(span_u8_slice_tail(tail, 2), cast(u64, ptr));
    sink->pos += fmt_ptr_length;
}

static void
log_sink_format_span_s64(LogSink* sink, span_s64 s) {
    log_sink_put_byte(sink, '[');
    if (s.len == 0) {
        log_sink_put_byte(sink, ']');
        return;
    }

    log_sink_format_dec_s64(sink, s.ptr[0]);
    for (uint i = 1; i < s.len; i += 1) {
        log_sink_put_byte(sink, ',');
        log_sink_put_space(sink);
        log_sink_format_dec_s64(sink, s.ptr[i]);
    }
    log_sink_put_byte(sink, ']');
}

#define LOG_FIELD_U64 0
#define LOG_FIELD_S64 1
#define LOG_FIELD_STR 2
#define LOG_FIELD_PTR 3

#define LOG_FIELD_SPAN_S64 4

typedef union {
    u64 u64;
    s64 s64;
    str str;

    span_s64 span_s64;

    void* ptr;
} LogFieldValue;

typedef struct {
    LogFieldValue value;

    str name;
    u8  kind;
} LogField;

typedef struct {
    LogField* ptr;
    uint len;
} SpanLogField;

static SpanLogField
make_span_log_field(LogField* ptr, uint len) {
    SpanLogField s = {};
    if (len == 0) {
        return s;
    }

    s.ptr = ptr;
    s.len = len;
    return s;
}

static void
log_sink_format_field_value(LogSink* sink, u8 kind, LogFieldValue value) {
    switch (kind) {
    case LOG_FIELD_U64:
        log_sink_format_dec_u64(sink, value.u64);
        return;
    case LOG_FIELD_S64:
        log_sink_format_dec_s64(sink, value.s64);
        return;
    case LOG_FIELD_STR:
        log_sink_format_str(sink, value.str);
        return;
    case LOG_FIELD_PTR:
        log_sink_format_ptr(sink, value.ptr);
        return;
    case LOG_FIELD_SPAN_S64:
        log_sink_format_span_s64(sink, value.span_s64);
        return;
    default:
        panic_trap();
    }
}

static void
log_sink_format_field(LogSink* sink, LogField field) {
    log_sink_write(sink, field.name);
    log_sink_put_byte(sink, ':');
    log_sink_put_space(sink);
    log_sink_format_field_value(sink, field.kind, field.value);
}

static void
log_sink_format_message_field(LogSink* sink, LogField field) {
    log_sink_put_byte(sink, '{');
    log_sink_format_field(sink, field);
    log_sink_put_byte(sink, '}');
}

static void
log_sink_format_message_fields(LogSink* sink, SpanLogField fields) {
    must(fields.len != 0);

    log_sink_put_byte(sink, '{');
    log_sink_format_field(sink, fields.ptr[0]);
    for (uint i = 1; i < fields.len; i += 1) {
        log_sink_put_byte(sink, ',');
        log_sink_put_space(sink);
        log_sink_format_field(sink, fields.ptr[i]);
    }
    log_sink_put_byte(sink, '}');
}

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

static LogField
log_field_span_s64(str name, span_s64 value) {
    LogField field = {};
    field.value.span_s64 = value;
    field.name = name;
    field.kind = LOG_FIELD_SPAN_S64;
    return field;    
}

typedef struct {
    // Short descriptive name of the logger.
    // It will be automatically added to every log message.
    str name;

    LogSink* sink;

    u8 level;
} Logger;

/*/doc

Do not reorder elements in this array. It is tied to log level constants.
*/
static const str
log_prefix_table[] = {
    sl("[fatal] "),
    sl("[error] "),
    sl(" [warn] "),
    sl(" [info] "),
    sl("[debug] "),
};

static void
init_log(Logger* lg, LogSink* sink, u8 level) {
    lg->name = empty_str;
    lg->sink = sink;
    lg->level = level;
}

/*/doc

Create a new named logger with the same sink and level as the given one.
*/
static Logger
log_spawn(Logger *lg, str name) {
    Logger l = {};
    l.name = name;
    l.sink = lg->sink;
    l.level = lg->level;
    return l;
}

static void
log_message(Logger* lg, u8 level, str s) {
    if (level > lg->level) {
        return;
    }

    log_sink_format_clock(lg->sink);
    log_sink_write(lg->sink, log_prefix_table[level]);
    log_sink_format_logger_name(lg->sink, lg->name);
    log_sink_write(lg->sink, s);
    log_sink_put_newline(lg->sink);
}

static void
log_message_field(Logger* lg, u8 level, str s, LogField field) {
    if (level > lg->level) {
        return;
    }

    log_sink_format_clock(lg->sink);
    log_sink_write(lg->sink, log_prefix_table[level]);
    log_sink_format_logger_name(lg->sink, lg->name);
    log_sink_write(lg->sink, s);
    log_sink_put_space(lg->sink);
    log_sink_format_message_field(lg->sink, field);
    log_sink_put_newline(lg->sink);
}

static void
log_message_field2(Logger* lg, u8 level, str s, LogField field1, LogField field2) {
    if (level > lg->level) {
        return;
    }

    LogField fields[2] = { field1, field2 };

    log_sink_format_clock(lg->sink);
    log_sink_write(lg->sink, log_prefix_table[level]);
    log_sink_format_logger_name(lg->sink, lg->name);
    log_sink_write(lg->sink, s);
    log_sink_put_space(lg->sink);
    log_sink_format_message_fields(lg->sink, make_span_log_field(fields, 2));
    log_sink_put_newline(lg->sink);
}

static void
log_message_field3(Logger* lg, u8 level, str s, LogField field1, LogField field2, LogField field3) {
    if (level > lg->level) {
        return;
    }

    LogField fields[3] = { field1, field2, field3 };

    log_sink_format_clock(lg->sink);
    log_sink_write(lg->sink, log_prefix_table[level]);
    log_sink_format_logger_name(lg->sink, lg->name);
    log_sink_write(lg->sink, s);
    log_sink_put_space(lg->sink);
    log_sink_format_message_fields(lg->sink, make_span_log_field(fields, 3));
    log_sink_put_newline(lg->sink);
}

static void
log_message_fields(Logger* lg, u8 level, str s, SpanLogField fields) {
    if (level > lg->level) {
        return;
    }

    log_sink_format_clock(lg->sink);
    log_sink_write(lg->sink, log_prefix_table[level]);
    log_sink_format_logger_name(lg->sink, lg->name);
    log_sink_write(lg->sink, s);
    log_sink_put_space(lg->sink);
    log_sink_format_message_fields(lg->sink, fields);
    log_sink_put_newline(lg->sink);
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
log_debug_field2(Logger* lg, str s, LogField field1, LogField field2) {
    log_message_field2(lg, LOG_LEVEL_DEBUG, s, field1, field2);
}

static void
log_debug_fields(Logger* lg, str s, SpanLogField fields) {
    log_message_fields(lg, LOG_LEVEL_DEBUG, s, fields);
}

static void
log_info_field(Logger* lg, str s, LogField field) {
    log_message_field(lg, LOG_LEVEL_INFO, s, field);
}

static void
log_info_fields(Logger* lg, str s, SpanLogField fields) {
    log_message_fields(lg, LOG_LEVEL_INFO, s, fields);
}

static void
log_warn_field(Logger* lg, str s, LogField field) {
    log_message_field(lg, LOG_LEVEL_WARN, s, field);
}

static void
log_warn_fields(Logger* lg, str s, SpanLogField fields) {
    log_message_fields(lg, LOG_LEVEL_WARN, s, fields);
}

static void
log_error_field(Logger* lg, str s, LogField field) {
    log_message_field(lg, LOG_LEVEL_ERROR, s, field);
}

static void
log_error_field2(Logger* lg, str s, LogField field1, LogField field2) {
    log_message_field2(lg, LOG_LEVEL_ERROR, s, field1, field2);
}

static void
log_error_field3(Logger* lg, str s, LogField field1, LogField field2, LogField field3) {
    log_message_field3(lg, LOG_LEVEL_ERROR, s, field1, field2, field3);
}

static void
log_error_fields(Logger* lg, str s, SpanLogField fields) {
    log_message_fields(lg, LOG_LEVEL_ERROR, s, fields);
}
