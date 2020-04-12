#pragma once

#define CS_TYPE_PROCESS          1
#define CS_TYPE_KEY              2
#define CS_TYPE_ACTIVATE         3
#define CS_TYPE_TERMINATE        4
#define CS_TYPE_WIDGET           5
#define CS_TYPE_MOUSE            6
#define CS_TYPE_REDRAW           7


#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>

#include <cserv/widgets/widget.h>

class CSPacket{
	public:
		CSPacket(int type);
		~CSPacket();
		uint8_t* GetBuffer();
		int GetType();
		static CSPacket* CreatePacket(int type);
	private:
		int type;
		uint8_t buffer[128]; 
};


class CSProcess{
	public:
		CSProcess(pid_t pid);
		void ProcessEvents();
		void AddWidget(CSWidget* widget);
		static CSProcess* CreateProcess(pid_t pid);
		pid_t GetPid();
		bool ApplyFilter(CSPacket* packet);
		void SetupFilter(bool(*filter)(CSPacket*));
	private:
		pid_t pid;
		std::vector<CSWidget*> widgets;
		bool (*packet_filter)(CSPacket*);
		int canvas_size_x;
		int canvas_size_y;
		int canvas_offs_x; // Canvas offset from absolute (0,0)
		int canvas_offs_y;
};


class CServer{
	public:
		static int Init(const char* pipe);
		static void C_SendPacket(CSPacket* packet);
		static CSPacket* S_LastPacket(); 
		static CSPacket* C_LastPacket(); 
		static FILE* GetServerPipe();
		static FILE* GetSocket();
		static void S_AddProcess(pid_t proc);
		static CSProcess* S_GetProcess(pid_t pid);
		static std::vector<CSProcess*> S_GetAllProcesses();
		static void S_Tick();
		static FILE* OpenSocket(pid_t pid);
		static int C_InitClient();
	private:
		static FILE* server_pipe;
		static FILE* client_pipe;
		static std::vector<CSProcess*> processes;
};

