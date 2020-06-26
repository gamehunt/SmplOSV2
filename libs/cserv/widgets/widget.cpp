#include <cserv/widgets/widget.h>
#include <cserv/cserv.h>

int CSWidget::GetX(){
	return x;
}

int CSWidget::GetY(){
	return y;
}

CSWidget::~CSWidget(){;}

CSWidget::CSWidget(int _x,int _y,int _sx,int _sy,CSContext* context){
	x=_x;
	y=_y;
	sx=_sx;
	sy=_sy;
	ctx = context;
}

void CSWidget::SetX(int x){
	this->x = x;
}

void CSWidget::SetY(int y){
	this->y = y;
}

void CSWidget::SetSizeX(int sx){
	this->sx = sx;
}

void CSWidget::SetSizeY(int sy){
	this->sy = sy;
}

void CSWidget::AddChild(CSWidget* child){
	childs.push_back(child);
}

void CSWidget::SetParent(CSWidget* parnt)
{
	parent = parnt;
}


CSContext* CSWidget::GetContext(){
	return ctx;
}

void CSWidget::Render(){
	this->Draw();
	for(CSWidget* w : childs){
		w->Render();
	}
}
