/*/doc

This parser is meant to be used for testing facilities.
*/
typedef struct {
    str text;

    // Start position for next line scan.
    uint pos; 
} GenStackMachineTokenParser;

static str
gsm_parser_get_next_line(GenStackMachineTokenParser* p) {
    str tail = str_slice_tail(p->text, p->pos);
    RetIndex r = str_index_byte(tail, '\n');
    if (!r.ok) {
        p->pos = p->text.len;
        return tail;
    }

    str line = str_slice_head(s, r.index);
    p->pos += line.len + 1;
    return line;
}

static ErrorCode
gsm_parse_token(str s, GenStackMachineToken* t) {
    return 0;
}

#define ERROR_GSM_END 11 // TODO: make proper error code here

/*/doc

Parses next token from listing.

Listing is a human-readable text representaion of token stream.
*/
static ErrorCode
gsm_parser_get_next_token(GenStackMachineTokenParser *p, GenStackMachineToken* t) {
    while (p->pos < p->text.len) {
        str line = gsm_parser_get_next_line(p);
        line = str_trim_space(line)
        if (line.len == 0) {
            continue;
        }

        return gsm_parse_token(line, t);
    }
    return ERROR_GSM_END;
}

/*/doc

Do not reorder elements in this array. It is tied to token kind constants.
*/
static const str
gsm_token_string_table[] = {
    sl("PUT_LIT"),
    sl("PUT_REF"),
};

static void
unsafe_fmt_buffer_put_gsm_token_put_lit(FormatBuffer* buf, GenStackMachineToken* t) {
    unsafe_fmt_buffer_put_byte(buf, '"');
    unsafe_fmt_buffer_put_str(buf, t->data.put_lit.lit);
    unsafe_fmt_buffer_put_byte(buf, '"');
}

static void
unsafe_fmt_buffer_put_gsm_token_put_ref(FormatBuffer* buf, GenStackMachineToken* t) {
    unsafe_fmt_buffer_put_byte(buf, '(');
    unsafe_fmt_buffer_put_dec_u64(buf, t->data.put_ref.offset);
    unsafe_fmt_buffer_put_byte(buf, ',');
    unsafe_fmt_buffer_put_space(buf);
    unsafe_fmt_buffer_put_dec_u64(buf, t->data.put_ref.len);
    unsafe_fmt_buffer_put_byte(buf, ')');
}

static void
unsafe_fmt_buffer_put_gsm_token_data(FormatBuffer* buf, GenStackMachineToken* t) {
    switch (t->kind) {
    case GSM_TOKEN_PUT_LIT:
        unsafe_fmt_buffer_put_gsm_token_put_lit(m, t);
        break;
    case GSM_TOKEN_PUT_REF:
        unsafe_fmt_buffer_put_gsm_token_put_ref(m, t);
        break;
    default:
        panic_trap();
    }
}

#define GSM_TOKEN_FORMAT_PAD 16

static void
unsafe_fmt_buffer_put_gsm_token(FormatBuffer* buf, GenStackMachineToken* t) {
    str kind_str = gsm_token_string_table[t.kind];
    must(kind_str.len < GSM_TOKEN_FORMAT_PAD);

    unsafe_fmt_buffer_put_str(buf, kind_str);
    unsafe_fmt_buffer_put_space_repeat(buf, GSM_TOKEN_FORMAT_PAD - kind_str.len);
    unsafe_fmt_buffer_put_gsm_token_data(buf, t);
}
