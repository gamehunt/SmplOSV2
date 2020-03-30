#include <string.h>

const char* strchr(const char* str, int c) {
    int i = 0;
    while (str[i] && str[i] != c) ++i;
    return c == str[i] ? (char*)str + i : NULL;
}
