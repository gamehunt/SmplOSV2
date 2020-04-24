#include <cserv/widgets/text_screen.h>
#include <sys/syscall.h>
#include <fb.h>

//Test widget, moves on key press or new packet received in cserver

void CSTextScreenWidget::Draw(){
	int lx = 0;
	int ly = 0;
	for(char c : text){
		if(ly >= sy){
			break;
		}
		fb_char(c,x+lx,y+ly,0x00FFFFFF,0x00000000);
		if(c == '\t'){
			lx += 30;
		}else if(c == '\n'){
			lx = 0;
			ly += 20;
		}else{
			lx+=10;
		}
		if(lx >= sx){
			lx = 0;
			ly += 20;
		}
	}
}

void CSTextScreenWidget::Update(void* data){
	const char* str = (const char*)data;
	text = str;
}
