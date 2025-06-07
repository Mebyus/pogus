/*/doc

Returns true if span of integers is sorted in ascending order.
*/
static bool
is_sorted_asc_s64(span_s64 s) {
    if (s.len < 2) {
        return true;
    }

    for (uint i = 1; i < s.len; i += 1) {
        s64 a = s.ptr[i - 1];
        s64 b = s.ptr[i];

        if (a > b) {
            return false;
        }
    }

    return true;
} 

static void
insert_sort_s64(span_s64 s) {
    if (s.len < 2) {
        return;
    }

    for (uint j = 0; j < s.len; j += 1) {
        s64 min = s.ptr[j];
        uint q = j; // index of min element

        for (uint i = j + 1; i < s.len; i += 1) {
            s64 x = s.ptr[i];
            
            if (x < min) {
                min = x;
                q = i;
            }
        }

        s64 c = s.ptr[j];
        s.ptr[j] = min;
        s.ptr[q] = c;
    }
}

static void
quick_sort_s64(span_s64 s) {
    if (s.len <= 16) {
        insert_sort_s64(s);
        return;
    }

    uint m = s.len >> 1;
    s64 c = s.ptr[0];
    s.ptr[0] = s.ptr[m];
    s.ptr[m] = c;

    uint j = 0;
    for (uint i = 1; i < s.len; i += 1) {
        if (s.ptr[i] < s.ptr[0]) {
            j += 1;
            
            c = s.ptr[i];
            s.ptr[i] = s.ptr[j];
            s.ptr[j] = c;
        }
    }

    c = s.ptr[0];
    s.ptr[0] = s.ptr[j];
    s.ptr[j] = c;

    quick_sort_s64(span_s64_slice_head(s, j));
    quick_sort_s64(span_s64_slice_tail(s, j + 1));
}
