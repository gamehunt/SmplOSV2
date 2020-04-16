#include <cserv/cserv.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

FILE* CServer::server_pipe;
FILE* CServer::client_pipe;
std::vector<CSProcess*> CServer::processes;

int CServer::Init(const char* path){
	server_pipe = fopen(path,"r");
	if(!server_pipe){
		sys_pipe((char*)path,4096);
	}else{
		fclose(server_pipe);
	}
	server_pipe = fopen(path,"r+");
	if(!server_pipe){
		sys_echo("[CSRV] Failed to open server!\n");
		return 1;
	}
	char sock_path[128];
	sprintf(sock_path,"/proc/%d/cssock",getpid());
	client_pipe = fopen(sock_path,"r");
	if(!client_pipe){
		sys_pipe(sock_path,4096);
	}
	client_pipe = fopen(sock_path,"r");
	if(!client_pipe){
		sys_echo("[CSRV] Failed to open socket!\n");
		return 2;
	}
	return 0;
}

void CServer::C_SendPacket(CSPacket* packet){
		if(server_pipe){
			if(!fwrite(packet,sizeof(CSPacket),1,server_pipe)){
				//printf("Failed to send packet: unknown write failure\n");
				sys_echo("[CSRV] Failed to send packet: unknown write failure\n");
			}
		}else{
				//printf("Failed to send packet: cserver not available\n");
				sys_echo("[CSRV] Failed to send packet: cserver not available\n");
		}
}

FILE* CServer::GetServerPipe(){
	return server_pipe;
}
FILE* CServer::GetSocket(){
	return client_pipe;
}

CSProcess* CServer::S_GetProcess(pid_t pid){
	for(CSProcess* proc : processes){
		if(proc->GetPid() == pid){
			return proc;
		}
	}
	return 0;
}

void CServer::S_AddProcess(pid_t pid){
	if(!S_GetProcess(pid)){
		CSProcess* prc = CSProcess::CreateProcess(pid);
		processes.push_back(prc);
	}
}
		
std::vector<CSProcess*> CServer::S_GetAllProcesses(){
	return processes;
}

void CServer::S_Tick(){
	for(CSProcess* proc : processes){
		proc->ProcessEvents();
	}
}

FILE* CServer::OpenSocket(pid_t pid){
		char buff[128];
		sprintf(buff,"/proc/%d/cssock",pid);
		return fopen(buff,"w");
}

CSPacket* CServer::S_LastPacket(){
	CSPacket* packet = (CSPacket*)malloc(sizeof(CSPacket));
	if(fread(packet,sizeof(CSPacket),1,server_pipe)){
		return packet;
	}
	free(packet);
	return 0;
}

CSPacket* CServer::C_LastPacket(){
	CSPacket* packet = (CSPacket*)malloc(sizeof(CSPacket));
	char buff[128];
	sprintf(buff,"/proc/%d/cssock",getpid());
	FILE* sock = fopen(buff,"r");
	if(fread(packet,sizeof(CSPacket),1,sock)){
		return packet;
	}
	free(packet);
	return 0;
}

static void cserver_atexit_handlr(){
	sys_echo("[CSRV] Sending CS_TYPE_TERMINATE\n");
}

int CServer::C_InitClient(){
	if(CServer::Init("/dev/cserver")){
		return 1;
	}
	CSPacket* pack = CSPacket::CreatePacket(CS_TYPE_PROCESS);
	((pid_t*)pack->GetBuffer())[0] = getpid();
	CServer::C_SendPacket(pack);
	std::atexit(cserver_atexit_handlr);
	sys_echo("[CSRV] Client initialized\n");
	return 0;
}

CSPacket::CSPacket(int type){
	this->type = type;
	memset(this->buffer,0,128);
}
CSPacket::~CSPacket(){}
uint8_t* CSPacket::GetBuffer(){
	return buffer;
}

int CSPacket::GetType(){
	return type;
}

CSPacket* CSPacket::CreatePacket(int t){
	return new CSPacket(t);
}

CSProcess::CSProcess(pid_t pid){
	this->pid = pid;
	this->packet_filter = [](CSPacket* p){return false;};
}

void CSProcess::ProcessEvents(){
	for(CSWidget* w : this->widgets){
		w->Draw();
	}
}
CSProcess* CSProcess::CreateProcess(pid_t p){
	return new CSProcess(p);
}

pid_t CSProcess::GetPid(){
	return pid;
}

bool CSProcess::ApplyFilter(CSPacket* packet){
	return packet_filter(packet);
}
void CSProcess::SetupFilter(bool(*filter)(CSPacket*)){
	this->packet_filter = filter;
}

void CSProcess::AddWidget(CSWidget* w){
	widgets.push_back(w);
}
