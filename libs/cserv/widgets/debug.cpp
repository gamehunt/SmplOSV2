#include <cserv/widgets/debug.h>
#include <sys/syscall.h>
#include <fb.h>

//Test widget, moves on key press or new packet received in cserver

void CSDebugWidget::Draw(){
	for(int lx=0;lx<100;lx+=10){
		for(int ly=0;ly<50;ly+=17){
			fb_char('X',GetX()+lx,GetY()+ly,0x00000000,0x0000FF00);
		}
	}
	this->SetX(this->GetX() + 10);
	if(this->GetX() > 1024){
		this->SetX(0);
	}
}

