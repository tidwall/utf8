// Copyright 2026 Joshua J Baker. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
//
// https://github.com/tidwall/utf8
#ifndef UTF8_H
#define UTF8_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

size_t utf8_next(const void *data, size_t len, uint32_t *cp, bool *valid);
size_t utf8_last(const void *data, size_t len, uint32_t *cp, bool *valid);
bool utf8_valid(const void *data, size_t len);
size_t utf8_put(void *data, uint32_t cp);

size_t utf8_next_nocheck(const void *data, size_t len, uint32_t *cp);
size_t utf8_last_nocheck(const void *data, size_t len, uint32_t *cp);

#endif
