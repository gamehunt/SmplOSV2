#include <cserv/widgets/text_screen.h>
#include <sys/syscall.h>
#include <fb.h>
#include <cstring>

void CSTextScreenWidget::Draw(){
	int lx = 0;
	int ly = 0;
	for(char c : text){
	//	sys_echo("%d %d\n",sx,sy);
		if(ly >= sy){
			break;
		}
		if(c == '\t'){
			lx += 30;
			if(lx >= sx){
				lx = 0;
				ly += 20;
				if(ly >= sy){
					break;
				}
			}
		}else if(c == '\n'){
			lx = 0;
			ly += 20;
			if(ly >= sy){
				break;
			}
		}else{
			lx+=10;
			if(lx >= sx){
				lx = 0;
				ly += 20;
				if(ly >= sy){
					break;
				}
			}
			fb_char(c,x+lx,y+ly,0x00FFFFFF,0x00000000);
		}
		
	}
}

void CSTextScreenWidget::Update(void* data){
	uint8_t type = *(uint8_t*)data;
	const char* str = (const char*)((uint8_t*)data+1);
	//sys_echo("%d %s\n",type,str);
	if(type){
		text = str;
	}else{
		text += str;
	}
}
