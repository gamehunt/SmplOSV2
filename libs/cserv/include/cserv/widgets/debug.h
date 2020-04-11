#include <cserv/widgets/widget.h>

class CSDebugWidget:public CSWidget{
	public:
		CSDebugWidget(int x,int y):CSWidget(x,y){;}
		virtual void Draw();
};
