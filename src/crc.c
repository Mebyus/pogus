// IEEE is by far and away the most common CRC-32 polynomial.
// Used by ethernet (IEEE 802.3), v.42, fddi, gzip, zip, png, ...
#define CRC_IEEE 0xEDB88320

static u32
crc_table[256];

// TODO: maybe we should just fill {crc_table} with constants instead
// of requiring explicit initialization.
static void
init_crc_table() {
    for (u32 i = 0; i < 256; i += 1) {
        u32 c = i;
        for (uint j = 0; j < 8; j += 1) {
            if ((c & 1) == 1) {
                c = CRC_IEEE ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        crc_table[i] = c;
    }
}

/*/doc

Calculates CRC from a given {crc} value and raw bytes data.
One can call this function several times on different portions of data
to calculate resulting CRC.

u32 c = crc_digest(0, s1);
c = crc_digest(c, s2);
c = crc_digest(c, s3);

Variable {c} now contains CRC of {s1} + {s2} + {s3} concatenated.

You must call {init_crc_table()} once to use this function.
*/
static u32
crc_digest(u32 crc, span_u8 s) {
    must(crc_table[1] != 0); // enshure that static table is initialized

    u32 c = ~crc;
    for (uint i = 0; i < s.len; i += 1) {
        u8 k = cast(u8, c & 0xFF) ^ s.ptr[i];
        c = crc_table[k] ^ (c >> 8);
    }
    return ~c;
}
