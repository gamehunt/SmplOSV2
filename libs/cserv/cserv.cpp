#include <cserv/cserv.h>
#include <unistd.h>

FILE* CServer::pipe;
std::vector<CSProcess*> CServer::processes;

void CServer::Init(const char* path){
	pipe = fopen("/dev/cserver","r+");
	if(!pipe){
		sys_pipe("/dev/cserver",4096);
	}
	pipe = fopen("/dev/cserver","r+");
	if(!pipe){
		sys_echo("Failed to open server!",0);
	}
}

void CServer::SendPacket(CSPacket* packet){
	if(pipe){
		if(!fwrite(packet,sizeof(CSPacket),1,pipe)){
			printf("Failed to send packet: unknown write failure\n");
			sys_echo("Failed to send packet: unknown write failure",0);
		}
	}else{
			printf("Failed to send packet: cserver not available\n");
			sys_echo("Failed to send packet: cserver not available",0);
	}
}

FILE* CServer::GetPipe(){
	return pipe;
}

CSProcess* CServer::__s_GetProcess(pid_t pid){
	for(CSProcess* proc : processes){
		if(proc->GetPid() == pid){
			return proc;
		}
	}
	return 0;
}

CSPacket* CServer::__s_LastPacketForPid(pid_t pid){
	CSProcess* proc = __s_GetProcess(pid);
	if(proc && proc->GetPendingPackets().size()){
		CSPacket* pack = proc->GetPendingPackets().top();
		proc->GetPendingPackets().pop();
		return pack;
	}
	return 0;
}

void CServer::__s_AddProcess(pid_t pid){
	if(!__s_GetProcess(pid)){
		CSProcess* prc = CSProcess::CreateProcess(pid);
		char buff[128];
		sprintf(buff,"/proc/%d/cssock",pid);
		processes.push_back(prc);
	}
}
		
std::vector<CSProcess*> CServer::__s_GetAllProcesses(){
	return processes;
}

void CServer::__s_Tick(){
	for(CSProcess* proc : processes){
		proc->ProcessEvents();
	}
}

CSPacket* CServer::__s_LastPacket(){
	CSPacket* packet = (CSPacket*)malloc(sizeof(CSPacket));
	if(fread(packet,sizeof(CSPacket),1,pipe)){
		return packet;
	}
	free(packet);
	return 0;
}

CSPacket* CServer::LastPacket(){
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

CSPacket::CSPacket(int type,int buffer_size){
	this->type = type;
	this->buffer_size = buffer_size;
	if(this->buffer_size){
			this->buffer = malloc(buffer_size);
	}
}
CSPacket::~CSPacket(){
	if(this->buffer_size){
		delete buffer;
	}
}
void* CSPacket::GetBuffer(){
	return buffer;
}
int CSPacket::GetBuffSize(){
	return buffer_size;
}
int CSPacket::GetType(){
	return type;
}

CSPacket* CSPacket::CreatePacket(int t,int bs){
	return new CSPacket(t,bs);
}

CSProcess::CSProcess(pid_t pid){
	this->pid = pid;
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

void CSProcess::AddPacket(CSPacket* pack){
	pending_packets.push(pack);
}

std::stack<CSPacket*> CSProcess::GetPendingPackets(){
	return pending_packets;
}
