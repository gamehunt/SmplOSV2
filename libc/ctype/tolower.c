#include <ctype.h>

int tolower(int ch){
	return (ch >='A' && ch<='Z') ? (ch + 32) : (ch);    
}
