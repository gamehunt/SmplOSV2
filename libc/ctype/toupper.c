#include <ctype.h>

int toupper(int ch){
	return (ch >='a' && ch<='z') ? (ch - 32) : (ch);    
}
