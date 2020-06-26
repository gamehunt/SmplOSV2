#pragma once

#define CS_TYPE_PROCESS          1
#define CS_TYPE_KEY              2
#define CS_TYPE_ACTIVATE         3
#define CS_TYPE_TERMINATE        4
#define CS_TYPE_MOUSE            5


#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>

#include <cserv/widgets/widget.h>

#define POS(x,y) (CSContextPosition(x,y))

#define BUFFER(p,type,i) (((type*)p->GetBuffer())[i])

class CSPacket{
	public:
		CSPacket(int type);
		~CSPacket();
		uint8_t* GetBuffer();
		int GetType();
		uint8_t GetFlags();
		void SetFlags(uint8_t new_flags);
		static CSPacket* CreatePacket(int type);
	private:
		int type;
		uint8_t flags;
		uint8_t buffer[128]; 
};

class CSContextPosition{
	public:
		CSContextPosition(int x,int y);
		int x,y;
};

class CSContext{
	public:
		CSContext(int sx,int sy);
		uint32_t& operator[] (CSContextPosition const& pos);
		void restrict(int x,int y,int sx,int sy);
		void unrestrict();
		uint32_t* ToPlain();
		void FromPlain(uint32_t* plain);
		uint32_t GetX();
		uint32_t GetY();
		uint32_t GetSX();
		uint32_t GetSY();
		uint32_t GetOriginSX();
		uint32_t GetOriginSY();
		void clear();
	private:
		int rx,ry,rsx,rsy,sx,sy;
		uint32_t** canvas;
};

class CSProcess{
	public:
		CSProcess(pid_t pid);
		void SetCanvasProperties(int x,int y,int sx,int sy);
		static CSProcess* CreateProcess(pid_t pid,uint32_t x,uint32_t y,uint32_t sx,uint32_t sy);
		pid_t GetPid();
		bool ApplyFilter(CSPacket* packet);
		void SetupFilter(bool(*filter)(CSPacket*));
		uint32_t GetCanvasX();
		uint32_t GetCanvasY();
		uint32_t GetCanvasWidth();
		uint32_t GetCanvasHeight();
	private:
		pid_t pid;
		bool (*packet_filter)(CSPacket*);
		uint32_t pos_x;
		uint32_t pos_y;
		uint32_t canvas_size_x;
		uint32_t canvas_size_y;
};


class CServer{
	public:
		static int Init(const char* pipe);
		static void C_SendPacket(CSPacket* packet);
		static CSPacket* S_LastPacket(); 
		static CSPacket* C_LastPacket(); 
		static FILE* GetServerPipe();
		static FILE* GetSocket();
		static void S_AddProcess(CSProcess* prc);
		static CSProcess* S_GetProcess(pid_t pid);
		static std::vector<CSProcess*> S_GetAllProcesses();
		static FILE* OpenSocket(pid_t pid);
		static int C_InitClient(int sx,int sy);
		static CSContext* C_GetContext();
		static void RefreshScreen();
	private:
		static FILE* server_pipe;
		static FILE* client_pipe;
		static std::vector<CSProcess*> processes;
		static CSContext* c_ctx;
};

