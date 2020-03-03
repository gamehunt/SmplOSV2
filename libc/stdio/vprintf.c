#include <stdio.h>

char dtoc(int digit){
	if(digit < 10){
		return digit + '0';
	}else{
		switch(digit){
			case 10:
				return 'A';
			case 11:
				return 'B';
			case 12:
				return 'C';
			case 13:
				return 'D';
			case 14:
				return 'E';
			case 15:
				return 'F';
			default:
				return '\0';
		}
	}
}

void vprintf(const char* restrict format,va_list argptr){
	for(int i=0;i<strlen(format);i++){
		if(format[i] == '%'){
			i++;
			char tp = format[i];
			if(tp == 'c'){
				uint8_t arg = va_arg(argptr,uint8_t);
				putchar((char)arg);
			}
			if(tp == 'd'){
				uint32_t arg = va_arg(argptr,int);
				int d =0;
				char ds[32];
				do{
					ds[d] = dtoc(arg%10);
					arg/=10;
					d++;
				}while(arg != 0);
				for(int i=d-1;i>=0;i--){
					putchar(ds[i]);
				}
			}
			if(tp == 'l'){
				long arg = va_arg(argptr,long);
				int d =0;
				char ds[32];
				do{
					ds[d] = dtoc(arg%10);
					arg/=10;
					d++;
				}while(arg != 0);
				for(int i=d-1;i>=0;i--){
					putchar(ds[i]);
				}
			}
			if(tp == 'x'){
				int arg = va_arg(argptr,int);
				int d =0;
				char ds[32];
				do{
					ds[d] = dtoc(arg%16);
					arg/=16;
					d++;
				}while(arg != 0);
				for(int i=d-1;i>=0;i--){
					putchar(ds[i]);
				}
			}
			if(tp == 'e'){
				uint64_t arg = va_arg(argptr,uint64_t);
				int d =0;
				char ds[32];
				do{
					ds[d] = dtoc(arg%16);
					arg/=16;
					d++;
				}while(arg != 0);
				for(int i=d-1;i>=0;i--){
					putchar(ds[i]);
				}
			}
			if(tp == 'a'){
				uint32_t arg = va_arg(argptr,uint32_t);
				int d =0;
				char ds[32];
				do{
					ds[d] = dtoc(arg%16);
					arg/=16;
					d++;
				}while(arg != 0);
				puts("0x");
				for(int i=d-1;i>=0;i--){
					putchar(ds[i]);
				}
			}
			if(tp == 'b'){
				uint32_t arg = va_arg(argptr,uint32_t);
				int d =0;
				char ds[32];
				do{
					ds[d] = dtoc(arg%2);
					arg/=2;
					d++;
				}while(arg != 0);
				for(int i=d-1;i>=0;i--){
					putchar(ds[i]);
				}
			}
			if(tp == 's'){
				const char* arg = va_arg(argptr,const char*);
				puts(arg);
			}
		}else{
			putchar(format[i]);
			continue;
		}
	}
}
