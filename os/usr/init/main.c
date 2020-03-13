/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

int main(){
	asm("push 0");
	asm("int $0x7F");
	//int a = 0/0;
	while(1);
}
