/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>

static char dtoc(int digit){
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
				return 0;
		}
	}
}

int vsprintf(char* buf,const char* restrict format,va_list argptr){
	uint32_t buf_ptr = 0;	
	uint8_t size = 0; //shord or long
	uint8_t needarg = 0;
	for(int i=0;i<strlen(format);i++){
		if(format[i] == '%' || needarg){
			if(!needarg){
				i++;
			}
			char tp = format[i];
			if(tp == 'c'){
				char arg = (char)va_arg(argptr,int);
				buf[buf_ptr] = arg;
				buf_ptr++;
			}
			if(tp == 'd'){
				if(size == 0){
					int arg = va_arg(argptr,int);
					if(arg < 0){
						arg = -arg;
						buf[buf_ptr] = '-';
						buf_ptr++;
					}
					int d =0;
					char ds[32];
					do{
						ds[d] = dtoc(arg%10);
						arg/=10;
						d++;
					}while(arg != 0);
					for(int i=d-1;i>=0;i--){
						buf[buf_ptr] = ds[i];
						buf_ptr++;
					}
				}
				if(size == 1){
						short int arg = va_arg(argptr,short int);
						if(arg < 0){
							arg = -arg;
							buf[buf_ptr] = '-';
						buf_ptr++;
						}
						int d =0;
						char ds[32];
						do{
							ds[d] = dtoc(arg%10);
							arg/=10;
							d++;
						}while(arg != 0);
						for(int i=d-1;i>=0;i--){
							buf[buf_ptr] = ds[i];
							buf_ptr++;
						}
				}
				if(size == 2){
						char arg = (char)va_arg(argptr,int);
						if(arg < 0){
							arg = -arg;
							buf[buf_ptr] = '-';
						buf_ptr++;
						}
						int d =0;
						char ds[32];
						do{
							ds[d] = dtoc(arg%10);
							arg/=10;
							d++;
						}while(arg != 0);
						for(int i=d-1;i>=0;i--){
							buf[buf_ptr] = ds[i];
							buf_ptr++;
						}
				}
				if(size == 3){
						long int arg = va_arg(argptr,long int);
						if(arg < 0){
							arg = -arg;
							buf[buf_ptr] = '-';
						buf_ptr++;
						}
						int d =0;
						char ds[32];
						do{
							ds[d] = dtoc(arg%10);
							arg/=10;
							d++;
						}while(arg != 0);
						for(int i=d-1;i>=0;i--){
							buf[buf_ptr] = ds[i];
							buf_ptr++;
						}
				}
				if(size == 4){
						long long int arg = va_arg(argptr,long long int);
						if(arg < 0){
							arg = -arg;
							buf[buf_ptr] = '-';
						buf_ptr++;
						}
						int d =0;
						char ds[32];
						do{
							ds[d] = dtoc(arg%10);
							arg/=10;
							d++;
						}while(arg != 0);
						for(int i=d-1;i>=0;i--){
							buf[buf_ptr] = ds[i];
							buf_ptr++;
						}
				}
			}
			if(tp == 'u'){
				if(size == 0){
					unsigned int arg = va_arg(argptr,unsigned int);
					int d =0;
					char ds[32];
					do{
						ds[d] = dtoc(arg%10);
						arg/=10;
						d++;
					}while(arg != 0);
					for(int i=d-1;i>=0;i--){
						buf[buf_ptr] = ds[i];
						buf_ptr++;
					}
				}
				if(size == 1){
						unsigned short int arg = va_arg(argptr,unsigned short int);
						int d =0;
						char ds[32];
						do{
							ds[d] = dtoc(arg%10);
							arg/=10;
							d++;
						}while(arg != 0);
						for(int i=d-1;i>=0;i--){
							buf[buf_ptr] = ds[i];
							buf_ptr++;
						}
				}
				if(size == 2){
						unsigned char arg = (unsigned char)va_arg(argptr,int);
						int d =0;
						char ds[32];
						do{
							ds[d] = dtoc(arg%10);
							arg/=10;
							d++;
						}while(arg != 0);
						for(int i=d-1;i>=0;i--){
							buf[buf_ptr] = ds[i];
							buf_ptr++;
						}
				}
				if(size == 3){
						unsigned long int arg = va_arg(argptr,unsigned long int);
						int d =0;
						char ds[32];
						do{
							ds[d] = dtoc(arg%10);
							arg/=10;
							d++;
						}while(arg != 0);
						for(int i=d-1;i>=0;i--){
							buf[buf_ptr] = ds[i];
							buf_ptr++;
						}
				}
				if(size == 4){
						unsigned long long int arg = va_arg(argptr,unsigned long long int);
						int d =0;
						char ds[32];
						do{
							ds[d] = dtoc(arg%10);
							arg/=10;
							d++;
						}while(arg != 0);
						for(int i=d-1;i>=0;i--){
							buf[buf_ptr] = ds[i];
							buf_ptr++;
						}
				}
			}
			if(tp == 'x'){
				if(size == 0){
					unsigned int arg = va_arg(argptr,unsigned int);
					int d =0;
					char ds[32];
					do{
						ds[d] = dtoc(arg%16);
						arg/=16;
						d++;
					}while(arg != 0);
					for(int i=d-1;i>=0;i--){
						buf[buf_ptr] = ds[i];
						buf_ptr++;
					}
				}
				if(size == 1){
						unsigned short int arg = va_arg(argptr,unsigned short int);
						int d =0;
						char ds[32];
						do{
							ds[d] = dtoc(arg%16);
							arg/=16;
							d++;
						}while(arg != 0);
						for(int i=d-1;i>=0;i--){
							buf[buf_ptr] = ds[i];
						buf_ptr++;
						}
				}
				if(size == 2){
						unsigned char arg = (unsigned char)va_arg(argptr,int);
						int d =0;
						char ds[32];
						do{
							ds[d] = dtoc(arg%16);
							arg/=16;
							d++;
						}while(arg != 0);
						for(int i=d-1;i>=0;i--){
							buf[buf_ptr] = ds[i];
						buf_ptr++;
						}
				}
				if(size == 3){
						unsigned long int arg = va_arg(argptr,unsigned long int);
						int d =0;
						char ds[32];
						do{
							ds[d] = dtoc(arg%16);
							arg/=16;
							d++;
						}while(arg != 0);
						for(int i=d-1;i>=0;i--){
							buf[buf_ptr] = ds[i];
						buf_ptr++;
						}
				}
				if(size == 4){
						unsigned long long int arg = va_arg(argptr,unsigned long long int);
						int d =0;
						char ds[32];
						do{
							ds[d] = dtoc(arg%16);
							arg/=16;
							d++;
						}while(arg != 0);
						for(int i=d-1;i>=0;i--){
							buf[buf_ptr] = ds[i];
						buf_ptr++;
						}
				}
			}
			if(tp == 'e'){
				//Scientific notation
			}
			if(tp == 'p'){
				uint32_t arg = va_arg(argptr,uint32_t);
				int d =0;
				char ds[32];
				do{
					ds[d] = dtoc(arg%16);
					arg/=16;
					d++;
				}while(arg != 0);
				buf[buf_ptr] = '0';
				buf_ptr++;
				buf[buf_ptr] = 'x';
				buf_ptr++;
				for(int i=d-1;i>=0;i--){
					buf[buf_ptr] = ds[i];
						buf_ptr++;
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
					buf[buf_ptr] = ds[i];
						buf_ptr++;
				}
			}
			if(tp == 's'){
				const char* arg = va_arg(argptr,const char*);
				for(int i=0;i<strlen(arg);i++){
					buf[buf_ptr] = arg[i];
					buf_ptr++;
				}
			}
			if(tp == 'l'){
				if(size == 3){
					size++;
				}else{
					size = 3;
				}
				needarg = 1;
			}else if(tp == 'h'){
				if(size == 1){
					size++;
				}else{
					size = 1;
				}
				needarg = 1;
			}else{
				size = 0;
				needarg = 0;
			}
		}else{
			buf[buf_ptr] = format[i];
			buf_ptr++;
			continue;
		}
	}
	return buf_ptr;
}
