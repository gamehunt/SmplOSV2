#include <stdio.h>

char *fgets(char *str, int num, FILE *stream){
	int str_idx = 0;
	while(str_idx < num-1){
		while(!fread(&str[str_idx],1,1,stream));
		str_idx++;
		if(str[str_idx-1] == 0){
			break;
		}
	}
	str[str_idx] = '\0';
	return str;
}
