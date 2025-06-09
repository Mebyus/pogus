
// Common outcome for read-like operations. Other error codes
// should not squat this value.
#define ERROR_READER_EOF 1
#define ERROR_WRITER_EOF 2

/*
Describes result returned by any read-like operation.
*/
typedef struct {
    // Actual number of bytes read.
    //
    // Can be not zero even if error code is not zero.
    uint count;

    // Error code produced by read.
    ErrorCode code;
} RetRead;


/*
Describes result returned by any write-like operation.
*/
typedef struct {
    // Actual number of bytes written.
    //
    // Can be not zero even if error code is not zero.
    uint count;

    // Error code produced by write.
    ErrorCode code;
} RetWrite;

typedef RetRead (*BagFuncRead)(void*, span_u8);

typedef struct {
    uint type_id;

    /* Method table */
    BagFuncRead read;
} BagReaderTab;

typedef struct {
    // Holds the underlying type instance.
    void* ptr;

    const BagReaderTab* tab;
} Reader;

typedef RetWrite (*BagFuncWrite)(void*, span_u8);

typedef struct {
    uint type_id;

    /* Method table */
    BagFuncWrite write;
} BagWriterTab;

typedef struct {
    // Holds the underlying type instance.
    void* ptr;

    const BagWriterTab* tab;
} Writer;

typedef struct {
    // Array pointer to buffer memory.
    u8* ptr;

    // Number of bytes currently stored in buffer.
    uint pos;

    // Buffer capacity. Maximum number of bytes it can hold.
    uint cap;
} CapBuffer;

typedef struct {
    // Total number of bytes written. It may differ from number of bytes read
    // in case of error.
    //
    // Could be not 0 even if error occured.
    u64 count;

    ErrorCode code;
} RetCopy;

static RetCopy
bag_copy(Writer writer, Reader reader) {
    RetCopy ret = {};

    u8 array_buf[1 << 14];
    span_u8 buf = make_span_u8(array_buf, array_len(array_buf));
    while (true) {
        RetRead r = reader.tab->read(reader.ptr, buf);
        if (r.code != 0) {
            if (r.code != ERROR_READER_EOF) {
                ret.code = r.code;
                return ret;
            }
        }
        if (r.count == 0) {
            if (r.code == ERROR_READER_EOF) {
                return ret;
            }
            continue;
        }

        RetWrite w = writer.tab->write(writer.ptr, span_u8_slice_head(buf, r.count));
        ret.count += w.count;
        if (w.code != 0) {
            ret.code = w.code;
            return ret;
        }
        if (r.code == ERROR_READER_EOF) {
            return ret;
        }
    }
}

static RetWrite
cap_buffer_write(CapBuffer* buf, span_u8 s) {
    RetWrite ret = {};
    return ret;
}

const BagWriterTab cap_buffer_bag_writer_tab = {
    .type_id = 1,
    .write = cast(BagFuncWrite, cap_buffer_write),
};

static Writer
bag_cap_buffer(CapBuffer* buf) {
    must(buf != nil);

    Writer w = {};
    w.ptr = buf;
    w.tab = &cap_buffer_bag_writer_tab;
    return w;
}
