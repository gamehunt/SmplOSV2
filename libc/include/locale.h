#pragma once

#define LC_ALL      0
#define LC_COLLATE  1
#define LC_CTYPE    2
#define LC_MONETARY 3
#define LC_NUMERIC  4
#define LC_TIME     5

#include <cheader.h>

CH_START

struct lconv{
	
};

char *setlocale(int type, const char *locale);
struct lconv *localeconv(void);

CH_END
