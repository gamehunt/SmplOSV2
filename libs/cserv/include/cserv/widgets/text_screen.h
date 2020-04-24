#pragma once

#include <cserv/widgets/widget.h>

#include <string>

class CSTextScreenWidget:public CSWidget{
	public:
		CSTextScreenWidget(int x,int y):CSWidget(x,y){
			sx = 100;
			sy = 100;
		};
		virtual void Draw();
		virtual void Update(void* data);
	private:
		std::string text; 
};
