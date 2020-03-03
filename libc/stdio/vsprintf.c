/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>

void vsprintf(char* buf,const char* restrict format,va_list argptr){
	uint32_t buf_ptr = 0;	
	for(int i=0;i<strlen(format);i++){
		if(format[i] != '%'){
			buf[buf_ptr] = format[i];
			buf_ptr++;			
		}else{
			i++;
			char tp = format[i];
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
					buf[buf_ptr] = ds[i];
					buf_ptr++;
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
					buf[buf_ptr] = ds[i];
					buf_ptr++;
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
					buf[buf_ptr] = ds[i];
					buf_ptr++;
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
					buf[buf_ptr] = ds[i];
					buf_ptr++;
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
				buf[buf_ptr] = '0';
				buf[buf_ptr+1] = 'x';	
				buf_ptr+=2;
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
		}		
		
	}
}
