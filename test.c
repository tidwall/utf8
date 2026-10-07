// Copyright 2026 Joshua J Baker. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
//
// https://github.com/tidwall/utf8
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "utf8.h"

void test_ascii(void) {
    printf("Running ASCII tests...\n");
    uint8_t data[] = "A";
    uint32_t cp = 0;
    bool valid = false;
    
    size_t n = utf8_next(data, 1, &cp, &valid);
    assert(n == 1);
    assert(valid == true);
    assert(cp == 0x41); // 'A'
}

void test_valid_multibyte(void) {
    printf("Running valid multibyte tests...\n");
    uint32_t cp = 0;
    bool valid = false;
    size_t n;

    // 2-Byte: Cent sign (¢) -> U+00A2 -> C2 A2
    uint8_t u2[] = {0xC2, 0xA2};
    n = utf8_next(u2, 2, &cp, &valid);
    assert(n == 2);
    assert(valid == true);
    assert(cp == 0x00A2);

    // 3-Byte: Euro sign (€) -> U+20AC -> E2 82 AC
    uint8_t u3[] = {0xE2, 0x82, 0xAC};
    n = utf8_next(u3, 3, &cp, &valid);
    assert(n == 3);
    assert(valid == true);
    assert(cp == 0x20AC);

    // 4-Byte: Earth Globe (🌍) -> U+1F30D -> F0 9F 8C 8D
    uint8_t u4[] = {0xF0, 0x9F, 0x8C, 0x8D};
    n = utf8_next(u4, 4, &cp, &valid);
    assert(n == 4);
    assert(valid == true);
    assert(cp == 0x1F30D);
}

void test_overlong_vulnerabilities(void) {
    printf("Running security overlong encoding tests...\n");
    uint32_t cp = 0;
    bool valid = false;
    size_t n;

    // Invalid 2-byte overlong encoding of ASCII 'A' (0x41) -> C1 81
    uint8_t o2[] = {0xC1, 0x81};
    n = utf8_next(o2, 2, &cp, &valid);
    assert(valid == false);
    assert(cp == 0xFFFD); // Emitted replacement character
    assert(n == 1);       // Consumed exactly 1 broken byte

    // Invalid 3-byte overlong encoding of ASCII 'A' (0x41) -> E0 81 81
    uint8_t o3[] = {0xE0, 0x81, 0x81};
    n = utf8_next(o3, 3, &cp, &valid);
    assert(valid == false);
    assert(cp == 0xFFFD);
    assert(n == 1);

    // Invalid 4-byte overlong encoding of U+FFFF -> F0 8F BF BF
    uint8_t o4[] = {0xF0, 0x8F, 0xBF, 0xBF};
    n = utf8_next(o4, 4, &cp, &valid);
    assert(valid == false);
    assert(cp == 0xFFFD);
    assert(n == 1);
}

void test_surrogates_and_bounds(void) {
    printf("Running surrogate and structural bounds tests...\n");
    uint32_t cp = 0;
    bool valid = false;
    size_t n;

    // UTF-16 Lead Surrogate U+D800 encoded as 3-bytes -> ED A0 80
    uint8_t surrogate[] = {0xED, 0xA0, 0x80};
    n = utf8_next(surrogate, 3, &cp, &valid);
    assert(valid == false);
    assert(cp == 0xFFFD);
    assert(n == 1);

    // Beyond Max Unicode Limit: U+110000 -> F4 90 80 80
    uint8_t out_of_bounds[] = {0xF4, 0x90, 0x80, 0x80};
    n = utf8_next(out_of_bounds, 4, &cp, &valid);
    assert(valid == false);
    assert(cp == 0xFFFD);
    assert(n == 1);
}

void test_truncation_safety(void) {
    printf("Running stream truncation safety tests...\n");
    uint32_t cp = 0;
    bool valid = false;
    size_t n;

    // A 4-byte prefix cut off after 2 bytes
    uint8_t trunc[] = {0xF0, 0x9F}; 
    n = utf8_next(trunc, 2, &cp, &valid);
    assert(valid == false);
    assert(cp == 0xFFFD);
    assert(n == 1);
    
    // Empty stream check
    n = utf8_next(NULL, 0, &cp, &valid);
    assert(n == 0);
    assert(valid == false);
    assert(cp == 0xFFFD);
}

void test_reverse_parsing(void) {
    printf("Running reverse parsing (utf8_last) tests...\n");
    uint32_t cp = 0;
    bool valid = false;
    size_t n;

    // Stream: "A¢" (0x41 followed by C2 A2)
    uint8_t data[] = {0x41, 0xC2, 0xA2};
    n = utf8_last(data, 3, &cp, &valid);
    assert(n == 2); // Read 2 bytes from the end
    assert(valid == true);
    assert(cp == 0x00A2); // Successfully read the Cent sign

    // Stream ending in corrupted byte
    uint8_t bad_data[] = {0x41, 0xC2}; // Broken tail structure
    n = utf8_last(bad_data, 2, &cp, &valid);
    assert(valid == false);
    assert(cp == 0xFFFD);
}

void test_roundtrip_encoding(void) {
    printf("Running roundtrip code point encoding tests...\n");
    uint8_t buffer[4];
    size_t written;

    // Encode Euro Sign (U+20AC)
    memset(buffer, 0, 4);
    written = utf8_put(buffer, 0x20AC);
    assert(written == 3);
    assert(buffer[0] == 0xE2 && buffer[1] == 0x82 && buffer[2] == 0xAC);

    // Safety fallback: Attempting to encode an invalid surrogate safely falls back to U+FFFD
    memset(buffer, 0, 4);
    written = utf8_put(buffer, 0xD800); 
    assert(written == 3); // U+FFFD requires 3 bytes
    assert(buffer[0] == 0xEF && buffer[1] == 0xBF && buffer[2] == 0xBD);
}

int main(void) {
    test_ascii();
    test_valid_multibyte();
    test_overlong_vulnerabilities();
    test_surrogates_and_bounds();
    test_truncation_safety();
    test_reverse_parsing();
    test_roundtrip_encoding();
    printf("PASS\n");
    return 0;
}
