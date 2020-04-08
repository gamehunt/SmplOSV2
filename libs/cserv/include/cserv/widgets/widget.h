#pragma once

class CSWidget{
	public:
		virtual int GetX();
		virtual int GetY();
		virtual void Draw()=0;
		virtual ~CSWidget();
	private:
		int x,y;	
};
