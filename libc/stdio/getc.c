#include <stdio.h>

int getc(FILE *stream){
    char buff[1];
    fread(buff,1,1,stream);
    return buff[0];
}
