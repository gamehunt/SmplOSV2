#include <cserv/widgets/widget.h>

int CSWidget::GetX(){
	return x;
}

int CSWidget::GetY(){
	return y;
}

CSWidget::~CSWidget(){;}

CSWidget::CSWidget(int _x,int _y){
	x=_x;
	y=_y;
}

void CSWidget::SetX(int x){
	this->x = x;
}
void CSWidget::SetY(int y){
	this->y = y;
}
