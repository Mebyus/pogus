typedef struct {
    uint index;
    bool ok;
} RetIndex;

#define SMALL_STRING_PATTERN (1 << 10)

/*/doc

Argument {lps} must have the same length as string {p}.
Length must be at least 1.
*/
static void
unsafe_str_fill_longest_prefix_suffix(span_uint lps, str p) {
    // Length of the previous longest prefix suffix
    uint k = 0;

    lps.ptr[0] = 0;

    uint i = 1;
    while (i < p.len) {
        if (p.ptr[i] == p.ptr[k]) {
            k += 1;
            lps.ptr[i] = k;
            i += 1;
        } else {
            if (k != 0) {
                k = lps.ptr[k - 1];
            } else {
                lps.ptr[i] = 0;
                i += 1;
            }
        }
    }
}

/*/doc

Function uses {lps} argument as a buffer. Its length must be the same as {p} length.
Uses KMP (Knuth-Morris-Pratt) algorithm.
*/
static RetIndex
unsafe_str_index_kmp(str s, str p, span_uint lps) {
    unsafe_str_fill_longest_prefix_suffix(lps, p);
    
    uint i = 0; // index for {s}
    uint j = 0; // index for {p}
    
    RetIndex ret = {};
    while ((s.len - i) >= (p.len - j)) {
        if (s.ptr[i] == p.ptr[j]) {
            i += 1;
            j += 1;
        }

        if (j >= p.len) {
            // match found
            ret.index = i - j;
            ret.ok = true;
            return ret;
        }

        if (i < s.len && s.ptr[i] != p.ptr[j]) {
            if (j != 0) {
                j = lps.ptr[j - 1];
            } else {
                i += 1;
            }
        }
    }

    // no match found
    return ret;
}

static RetIndex
unsafe_str_index_small(str s, str p) {
    must(p.len <= SMALL_STRING_PATTERN);

    uint buf[SMALL_STRING_PATTERN];
    span_uint lps = make_span_uint(buf, p.len);
    return unsafe_str_index_kmp(s, p, lps);
}

/*/doc

Searches for a string pattern {p} inside another string {s}.
If match is found, returns index of the first match such that
s[i] == p[0] is true.

Argument {p} must not be an empty string.
*/
static RetIndex
str_index(str s, str p) {
    must(p.len != 0);

    RetIndex ret = {};
    if (p.len > s.len) {
        return ret;
    }

    if (p.len < SMALL_STRING_PATTERN) {
        return unsafe_str_index_small(s, p);
    }

    // TODO: code for patterns that are not small, probably need an allocator here
    return ret;
}
