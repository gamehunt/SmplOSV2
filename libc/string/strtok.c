#include <string.h>

char * strtok (char *str, const char *delim)
{
	static char * last = 0;
    if (str) last = str;
    if ((last == 0) || (*last == 0)) return 0;
    char * c = last;
    while(strchr(delim,*c)) ++c;
    if (*c == 0) return 0;
    char * start = c;
    while(*c && (strchr(delim,*c)==0)) ++c;
    if (*c == 0)
    {
        last = c;
        return start;
    }
    *c = 0;
    last = c+1;
    return start;
}
