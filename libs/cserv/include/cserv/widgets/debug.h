#pragma once

#include <cserv/widgets/widget.h>

class CSDebugWidget:public CSWidget{
	public:
		CSDebugWidget(int x,int y,int sx,int sy,CSContext* c):CSWidget(x,y,sx,sy,c){;}
		virtual void Draw();
};
