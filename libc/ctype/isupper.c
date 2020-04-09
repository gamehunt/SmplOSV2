#include <ctype.h>

int isupper(int ch)
{
    return (ch >= 'A' && ch <= 'Z');  // ASCII only - not a good implementation!
}
