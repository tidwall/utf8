# utf8

Simple C library for reading, writing, and validating UTF-8.

## Example (read UTF-8 data)

The `utf8_next()` and `utf8_last()` functions allow for reading UTF-8 encoded
data starting from the front or back or a string.

```c
char *str = "π ≈ 3.14 • 🔀 [CĘŚŪ-8: 🄽🄿] •𓀀𓀁𓀂• 🚀✨";
char *end = str+strlen(str);
while (str < end) {
    uint32_t cp; // the next code point
    size_t n = utf8_next(str, end-str, &cp, 0);
    printf("'%.*s'\t%zu byte\t0x%x\n", (int)n, str, n, cp);
    str += n;
}
printf("\n");
```

Output:

```c
'π'	2 byte	0x3c0
' '	1 byte	0x20
'≈'	3 byte	0x2248
' '	1 byte	0x20
'3'	1 byte	0x33
'.'	1 byte	0x2e
'1'	1 byte	0x31
'4'	1 byte	0x34
' '	1 byte	0x20
'•'	3 byte	0x2022
' '	1 byte	0x20
'🔀'	4 byte	0x1f500
' '	1 byte	0x20
'['	1 byte	0x5b
'C'	1 byte	0x43
'Ę'	2 byte	0x118
'Ś'	2 byte	0x15a
'Ū'	2 byte	0x16a
'-'	1 byte	0x2d
'8'	1 byte	0x38
':'	1 byte	0x3a
' '	1 byte	0x20
'🄽'	4 byte	0x1f13d
'🄿'	4 byte	0x1f13f
']'	1 byte	0x5d
' '	1 byte	0x20
'•'	3 byte	0x2022
'𓀀'	4 byte	0x13000
'𓀁'	4 byte	0x13001
'𓀂'	4 byte	0x13002
'•'	3 byte	0x2022
' '	1 byte	0x20
'🚀'	4 byte	0x1f680
'✨'	3 byte	0x2728
```

## API

Other functions include:

```c
size_t utf8_next(const void *data, size_t len, uint32_t *cp, bool *valid);
size_t utf8_last(const void *data, size_t len, uint32_t *cp, bool *valid);
bool utf8_valid(const void *data, size_t len);
size_t utf8_put(void *data, uint32_t cp);
size_t utf8_next_nocheck(const void *data, size_t len, uint32_t *cp);
size_t utf8_last_nocheck(const void *data, size_t len, uint32_t *cp);
size_t utf8_code_point_len(uint32_t cp);
```


