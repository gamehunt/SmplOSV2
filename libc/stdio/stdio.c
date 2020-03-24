#include <stdio.h>

FILE _stdout={
		.fd = 0
};

FILE _stderr={
		.fd = 1
};

FILE _stdin={
		.fd = 2
};

FILE* stdout = &_stdout;
FILE* stdin = &_stdin;
FILE* stderr = &_stderr;
