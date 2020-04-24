#pragma once

#define WIDGET_PACK_ADD 0
#define WIDGET_PACK_UPD 1
#define WIDGET_PACK_MOV 2
#define WIDGET_PACK_RES 3

class CSWidget{
	public:
		CSWidget(int x,int y);
		virtual int GetX();
		virtual int GetY();
		virtual void Draw()=0;
		virtual void Update(void* ctx)=0;
		virtual ~CSWidget();
		virtual void SetX(int x);
		virtual void SetY(int y);
		virtual void SetSizeX(int sx);
		virtual void SetSizeY(int sy);
	protected:
		int x,y;	
		int sx,sy;
};
