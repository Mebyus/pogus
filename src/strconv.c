typedef struct {
    // Contains parsed integer if {code} equals 0.
    u64 n;

    ErrorCode code;
} RetParseU64;

static bool
is_dec_digit(u8 x) {
    return ('0' <= x) && (x <= '9');
}

static u8
dec_digit_num(u8 x) {
    return x - '0';
}

#define ERROR_EMPTY_STRING       2
#define ERROR_BAD_INTEGER_FORMAT 3
#define ERROR_INTEGER_OVERFLOW   4

/*/doc

Parse u64 integer from a given string. It must not contain leading or
trailing spaces or any other non-decimal digit characters.
*/
static RetParseU64
parse_dec_u64(str s) {
    RetParseU64 ret = {};
    if (s.len == 0) {
        ret.code = ERROR_EMPTY_STRING;
        return ret;
    }
    if (s.len > max_u64_dec_length) {
        ret.code = ERROR_INTEGER_OVERFLOW;
        return ret;
    }

    u64 n = 0;
    uint i = 0;
    while (i < s.len) {
        u8 c = s.ptr[i];
        if (!is_dec_digit(c)) {
            ret.code = ERROR_BAD_INTEGER_FORMAT;
            return ret;
        }
        
        n = n * 10; // TODO: check for overflow in case when {s.len} is exactly  20
        n += dec_digit_num(c);
        i += 1;
    }

    ret.n = n;
    return ret;
}
