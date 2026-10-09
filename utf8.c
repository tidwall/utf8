// Copyright 2026 Joshua J Baker. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
//
// https://github.com/tidwall/utf8

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef UTF8_STATIC
#include "utf8.h"
#else
#define UTF8_EXTERN static
#endif

#ifndef UTF8_EXTERN
#define UTF8_EXTERN
#endif

#define UTF8_REPL 0xFFFD

static inline bool is_surrogate(uint32_t cp) {
    return cp >= 0xD800 && cp <= 0xDFFF;
}

static size_t _utf8_next_(const uint8_t *str, size_t len, uint32_t *cpo, 
    bool *ok)
{
    size_t n;
    uint32_t cp;
    uint32_t mincp;
    // The first byte has already been validated by the caller.
    if (str[0]>>5 == 6) {
        if (len < 2) goto fail;
        if (str[1]>>6 != 2) goto fail;
        cp = ((uint32_t)(str[0]&31)<<6)|((uint32_t)(str[1]&63));
        n = 2;
        mincp = 0x80;
    } else  if (str[0]>>4 == 14) {
        if (len < 3) goto fail;
        if (((str[1]>>6)|(str[2]>>6<<2)) != 10) goto fail;
        cp = ((uint32_t)(str[0]&15)<<12)|((uint32_t)(str[1]&63)<<6)|
             ((uint32_t)(str[2]&63));
        n = 3;
        mincp = 0x0800;
    } else if (str[0]>>3 == 30) {
        if (len < 4) goto fail;
        if (((str[1]>>6)|(str[2]>>6<<2)|(str[3]>>6<<4)) != 42) goto fail;
        cp = ((uint32_t)(str[0]&7)<<18)|((uint32_t)(str[1]&63)<<12)|
             ((uint32_t)(str[2]&63)<<6)|((uint32_t)(str[3]&63));
        n = 4;
        mincp = 0x010000;
    } else {
        goto fail;
    }
    if (cp < mincp) goto fail; // overlong validatation
    if (cp >= 0x10FFFF) goto fail; // restricted to utf-16
    if (is_surrogate(cp)) goto fail; // needs surrogate pairs
    *cpo = cp;
    *ok = true;
    return n;
fail:
    *cpo = UTF8_REPL;
    *ok = false;
    return 1;
}

// Parses the next utf8 code point from a data stream.
// Returns the code point and number of bytes read.
// Returns zero on utf8 error or if the len is zero.
UTF8_EXTERN
size_t utf8_next(const void *data, size_t len, uint32_t *cp, bool *valid) {
    const uint8_t *str = data;
    if (len > 0 && str[0] < 128) {
        if (cp) *cp = str[0];
        if (valid) *valid = true;
        return 1;
    }
    if (len == 0) {
        if (cp) *cp = UTF8_REPL;
        if (valid) *valid = false;
        return 0;
    }
    uint32_t cp0;
    bool ok0;
    size_t n = _utf8_next_(str, len, &cp0, &ok0);
    if (cp) *cp = cp0;
    if (valid) *valid = ok0;
    return n;
}

// Parses the next UTF-8 code point from a data stream.
// ASSUMPTION: 'data' and 'cp' are never NULL, and 'len' > 0.
UTF8_EXTERN
size_t utf8_next_nocheck(const void *data, size_t len, uint32_t *cp) {
    const uint8_t *str = data;
    if (str[0] < 128) {
        *cp = str[0];
        return 1;
    }
    bool ok;
    return _utf8_next_(str, len, cp, &ok);
}

static size_t _utf8_last_(const uint8_t *str, size_t len, uint32_t *cp,
    bool *ok)
{
    if (len == 0) {
        if (cp) *cp = UTF8_REPL;
        if (ok) *ok = true;
        return 0;
    }
    for (size_t i = len-1; ; i--) {
        if (str[i]>>6 == 3) {
            uint32_t cp0;
            bool ok0;
            size_t n = _utf8_next_(str+i, len-i, &cp0, &ok0);
            if (cp) *cp = cp0;
            if (ok) *ok = ok0;
            return n;
        }
        if (len-i == 4 || i == 0) break;
    }
    if (cp) *cp = UTF8_REPL;
    if (ok) *ok = false;
    return 1;
}

// Parses the last UTF-8 code point from a data stream.
UTF8_EXTERN
size_t utf8_last(const void *data, size_t len, uint32_t *cp, bool *valid) {
    const uint8_t *str = data;
    if (len > 0 && str[len-1] < 128) {
        if (cp) *cp = str[len-1];
        if (valid) *valid = true;
        return 1;
    }
    return _utf8_last_(str, len, cp, valid);
}

// Parses the last UTF-8 code point from a data stream.
// ASSUMTION: 'data' and 'cp' are never NULL, and 'len' > 0.
UTF8_EXTERN
size_t utf8_last_nocheck(const void *data, size_t len, uint32_t *cp) {
    const uint8_t *str = data;
    if (str[len-1] < 128) {
        *cp = str[len-1];
        return 1;
    }
    bool ok;
    return _utf8_last_(str, len, cp, &ok);
}

// Checks if provided data is valid UTF-8.
UTF8_EXTERN
bool utf8_valid(const void *data, size_t len) {
    const uint8_t *s = data;
    const uint8_t *e = s+len;
    while (s < e) {
        if (*s < 128) {
            s++;
        } else {
            uint32_t cp;
            bool ok;
            size_t n = _utf8_next_(s, e-s, &cp, &ok);
            if (!ok) return false;
            s += n;
        }
    }
    return true;
}

// Returns the number of bytes needed to encode a code point into UTF-8.
// Returns zero if the code point cannot be encoded.
UTF8_EXTERN
size_t utf8_code_point_len(uint32_t cp) {
    if (cp <= 127) return 1;
    if (cp <= 2047) return 2;
    if (is_surrogate(cp)) return 0;
    if (cp <= 65535) return 3;
    if (cp <= 0x10FFFF) return 4;
    return 0;
}

// Encodes a code point as UTF-8 to data and return the number of bytes written.
// Invalid code points are replaced with 0xFFFD (Unicode Replacement Character).
// ASSUMPTION: 'data' must not be NULL and large enough to store up to 4 bytes.
UTF8_EXTERN
size_t utf8_put(void *data, uint32_t cp) {
    uint8_t *str = data;
    if (cp < 128) {
        str[0] = cp;
        return 1;
    }
    if (cp < 2048) {
        str[0] = 192 | (cp>>6);
        str[1] = 128 | (cp&63);
        return 2;
    }
    if (is_surrogate(cp) || cp > 0x10FFFF) {
        // Error, use replacement character
        cp = UTF8_REPL;
    }
    if (cp < 65536) {
        str[0] = 224 | (cp>>12);
        str[1] = 128 | ((cp>>6)&63);
        str[2] = 128 | (cp&63);
        return 3;
    }
    str[0] = 240 | (cp>>18);
    str[1] = 128 | ((cp>>12)&63);
    str[2] = 128 | ((cp>>6)&63);
    str[3] = 128 | (cp&63);
    return 4;
}
