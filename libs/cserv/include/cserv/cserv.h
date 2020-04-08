#pragma once

#define CS_TYPE_PROCESS  1
#define CS_TYPE_KEY      2
#define CS_TYPE_ACTIVATE 3

#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>

#include <cserv/widgets/widget.h>

#include <vector>
#include <stack>

class CSPacket{
	public:
		CSPacket(int type,int buffer_size);
		virtual ~CSPacket();
		virtual void* GetBuffer();
		virtual int GetBuffSize();
		virtual int GetType();
		static CSPacket* CreatePacket(int type,int buffer_size);
	private:
		int type;
		void* buffer;
		int buffer_size;
};

class CSProcess{
	public:
		CSProcess(pid_t pid);
		void ProcessEvents();
		void AddWidget(CSWidget* widget);
		void AddPacket(CSPacket* pack);
		std::stack<CSPacket*> GetPendingPackets();
		static CSProcess* CreateProcess(pid_t pid);
		pid_t GetPid();
	private:
		pid_t pid;
		std::vector<CSWidget*> widgets;
		std::stack<CSPacket*>  pending_packets;
		int canvas_size_x;
		int canvas_size_y;
		int canvas_offs_x; // Canvas offset from absolute (0,0)
		int canvas_offs_y;
};

class CServer{
	public:
		static void Init(const char* pipe);
		static void SendPacket(CSPacket* packet);
		static CSPacket* __s_LastPacket(); //I need replace __s_ with some kind of ifdef
		static CSPacket* LastPacket();
		static CSPacket* __s_LastPacketForPid(pid_t pid);    
		static FILE* GetPipe();
		static void __s_AddProcess(pid_t proc);
		static CSProcess* __s_GetProcess(pid_t pid);
		static std::vector<CSProcess*> __s_GetAllProcesses();
		static void __s_Tick();
	private:
		static FILE* pipe;
		static std::vector<CSProcess*> processes;
};
