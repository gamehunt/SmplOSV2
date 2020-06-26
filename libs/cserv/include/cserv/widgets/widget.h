#pragma once

#define WIDGET_PACK_ADD 0
#define WIDGET_PACK_UPD 1
#define WIDGET_PACK_MOV 2
#define WIDGET_PACK_RES 3

//CLIENT ONLY
#include <vector>

class CSContext;

class CSWidget{
	public:
		CSWidget(int x,int y,int sx,int sy,CSContext* context);
		virtual int GetX();
		virtual int GetY();
		virtual void Draw()=0;
		void Render();
		virtual ~CSWidget();
		virtual void SetX(int x);
		virtual void SetY(int y);
		virtual void SetSizeX(int sx);
		virtual void SetSizeY(int sy);
		void AddChild(CSWidget* child);
		void SetParent(CSWidget* parent);
		CSContext* GetContext();
		//void SetContext(CSContext* context);
	protected:
		CSWidget* parent;
		int x,y;	
		int sx,sy;
		std::vector<CSWidget*> childs;
		CSContext* ctx;
};
