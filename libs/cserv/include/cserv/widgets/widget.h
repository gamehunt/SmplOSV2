#pragma once

class CSWidget{
	public:
		CSWidget(int x,int y);
		virtual int GetX();
		virtual int GetY();
		virtual void Draw()=0;
		virtual ~CSWidget();
		virtual void SetX(int x);
		virtual void SetY(int y);
	private:
		int x,y;	
};
