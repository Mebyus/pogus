#define PNG_MAGIC_LENGTH 8

static const u8
png_magic[PNG_MAGIC_LENGTH] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };

#define PNG_CHUNK_IHDR 0x49484452
#define PNG_CHUNK_IDAT 0x49444154
#define PNG_CHUNK_IEND 0x49454E44

typedef struct {
    span_u8 data;
    u32     type;
} PngChunk;

// returns number of bytes consumed.
static uint
png_read_chunk(span_u8 data, PngChunk *chunk) {
    const uint min_chunk_len = 4 + 4 + 0 + 4; // len + type + zero data + crc
    if (data.len < min_chunk_len) {
        return 0;
    }

    u32 len = unsafe_get_u32be(data.ptr);
    if (data.len < 12 + len) {
        // not enough bytes as declared in chunk length
        return 0;
    }

    u32 type = unsafe_get_u32be(data.ptr + 4);
    u32 crc = unsafe_get_u32be(data.ptr + 8 + len);
    chunk->data.ptr = data.ptr + 8;
    chunk->data.len = len;
    chunk->type = type;

    (void)(crc);

    return 12 + len;
}

static void
png_print_chunk_ihdr(span_u8 data) {
    if (data.len < 13) {
        return;
    }

    u32 width = unsafe_get_u32be(data.ptr);
    u32 height = unsafe_get_u32be(data.ptr + 4);
    u8 bit_depth = data.ptr[8];
    u8 color_type = data.ptr[9];
    u8 compression = data.ptr[10];
    u8 filter = data.ptr[11];
    u8 interlace = data.ptr[12];

    u8 buf_array[1 << 12];
    FormatBuffer buf;
    init_fmt_buffer(&buf, make_span_u8(buf_array, sizeof(buf_array)));
    
    unsafe_fmt_buffer_put_str(&buf, ss("width: "));
    unsafe_fmt_buffer_put_dec_u64(&buf, width);
    unsafe_fmt_buffer_put_newline(&buf);

    unsafe_fmt_buffer_put_str(&buf, ss("height: "));
    unsafe_fmt_buffer_put_dec_u64(&buf, height);
    unsafe_fmt_buffer_put_newline(&buf);

    unsafe_fmt_buffer_put_str(&buf, ss("bit depth: "));
    unsafe_fmt_buffer_put_dec_u64(&buf, bit_depth);
    unsafe_fmt_buffer_put_newline(&buf);

    unsafe_fmt_buffer_put_str(&buf, ss("color type: "));
    unsafe_fmt_buffer_put_dec_u64(&buf, color_type);
    unsafe_fmt_buffer_put_newline(&buf);

    unsafe_fmt_buffer_put_str(&buf, ss("compression: "));
    unsafe_fmt_buffer_put_dec_u64(&buf, compression);
    unsafe_fmt_buffer_put_newline(&buf);
    
    unsafe_fmt_buffer_put_str(&buf, ss("filter: "));
    unsafe_fmt_buffer_put_dec_u64(&buf, filter);
    unsafe_fmt_buffer_put_newline(&buf);
    
    unsafe_fmt_buffer_put_str(&buf, ss("interlace: "));
    unsafe_fmt_buffer_put_dec_u64(&buf, interlace);
    unsafe_fmt_buffer_put_newline(&buf);

    unsafe_fmt_buffer_put_newline(&buf);
    print(fmt_buffer_head(&buf));    
}

static void
png_print_chunk(PngChunk chunk) {
    u8 buf_array[1 << 12];
    FormatBuffer buf;
    init_fmt_buffer(&buf, make_span_u8(buf_array, sizeof(buf_array)));
    
    unsafe_fmt_buffer_put_str(&buf, ss("type: "));
    unsafe_fmt_buffer_put_hex_prefix_zeroes_u32(&buf, chunk.type);
    unsafe_fmt_buffer_put_newline(&buf);

    unsafe_fmt_buffer_put_str(&buf, ss("len: "));
    unsafe_fmt_buffer_put_dec_u64(&buf, chunk.data.len);
    unsafe_fmt_buffer_put_newline(&buf);
    
    print(fmt_buffer_head(&buf));

    switch (chunk.type) {
    case PNG_CHUNK_IHDR:
        png_print_chunk_ihdr(chunk.data);
        break;
    case PNG_CHUNK_IDAT:
        break;
    case PNG_CHUNK_IEND:
        // always has length 0
        break;
    default:
        // unknown chunk, skip inspection
    }
}

static void
png_print(span_u8 data) {
    if (!str_has_prefix(data, make_span_u8(cast(u8*, png_magic), PNG_MAGIC_LENGTH))) {
        // Error here
        return;
    }
    
    str tail = span_u8_slice_tail(data, PNG_MAGIC_LENGTH);
    while (tail.len != 0) {
        PngChunk chunk;
        uint n = png_read_chunk(tail, &chunk);

        if (n == 0) {
            break;
        }
        tail = span_u8_slice_tail(tail, n);
        png_print_chunk(chunk); // TODO: remove debug print

        if (chunk.type == PNG_CHUNK_IEND) {
            return;
        }
    }
}
