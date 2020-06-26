#include <cserv/widgets/debug.h>
#include <sys/syscall.h>
#include <cserv/cserv.h>
#include <fb.h>

void CSDebugWidget::Draw(){
	for(int i=0;i<500;i++){
		(*ctx)[CSContextPosition(100,i)] = 0x00FF0000;
	}
}

