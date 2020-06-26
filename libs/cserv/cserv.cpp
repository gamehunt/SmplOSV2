#include <cserv/cserv.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <fb.h>
#include <stdexcept>
#define SHARED_MEMORY_START      0xEA000000

FILE* CServer::server_pipe;
FILE* CServer::client_pipe;
std::vector<CSProcess*> CServer::processes;

CSContext* CServer::c_ctx;

CSContextPosition::CSContextPosition(int ax,int ay){
	x = ax;
	y = ay;
}

CSContext::CSContext(int _sx,int _sy){
	sx = _sx;
	sy = _sy;
	rsx = sx;
	rsy = sy;
	rx = 0;
	ry = 0;
	canvas = new uint32_t*[sy];
	for(int i=0;i<sy;i++){
		canvas[i] = new uint32_t[sx];
	}

}

uint32_t& CSContext::operator[] (CSContextPosition const& pos){
	int cy = ry + pos.y;
	int cx = rx + pos.x;
	if(!((cx >= rx && pos.x < rsx) && (cy >= ry && pos.y < rsy))){
		//sys_echo("OUT OF BOUND EXCEPTION THROWN! Position (%d;%d) is invalid for restriction %d;%d + %d;%d\n",pos.x,pos.y,rsx,rsy,rx,ry);
		throw std::out_of_range("CSContext::operator[]:invalid position specified!");
	}
	//sys_echo("%d %d\n",cx,cy);
	return canvas[cy][cx];
}

void CSContext::restrict(int x,int y,int sxr,int syr){
		rx = x;
		ry = y;
		rsx = sxr;
		rsy = syr;	
}

void CSContext::unrestrict(){
		rsx = sx;
		rsy = sy;
		rx = 0;
		ry = 0;
}

uint32_t CSContext::GetX(){
	return rx;
}

uint32_t CSContext::GetY(){
	return ry;		
}

uint32_t CSContext::GetSX(){
	return rsx;	
}

uint32_t CSContext::GetSY(){
	return rsy;
}

uint32_t CSContext::GetOriginSX(){
	return sx;
}

uint32_t CSContext::GetOriginSY(){
	return sy;
}

uint32_t* CSContext::ToPlain(){
	uint32_t* plain_buffer = new uint32_t[rsx*rsy];
	for(int i=0;i<rsx;i++){
		for(int j=0;j<rsy;j++){
			plain_buffer[j*rsx + i] = (*this)[CSContextPosition(i,j)];
		}
	}
	return plain_buffer;
}

void CSContext::clear(){
	for(int i=0;i<rsx;i++){
		for(int j=0;j<rsy;j++){
			(*this)[CSContextPosition(i,j)] = 0x00000000;
		}
	}
}

void CSContext::FromPlain(uint32_t* plain){
	for(int i=0;i<rsx;i++){
		for(int j=0;j<rsy;j++){
			(*this)[CSContextPosition(i,j)] = plain[j*rsx + i];
		}
	}
}

int CServer::Init(const char* path){
	server_pipe = fopen(path,"r");
	if(!server_pipe){
		sys_pipe((char*)path,1000*132);
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
				sys_echo("[CSRV] Failed to send packet: unknown write failure in %d; Server overloaded?\n",getpid());
				sys_echo("[CSRV] Server_pipe: %d\n",server_pipe->fd);
			}
			rewind(server_pipe);
		}else{
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

void CServer::S_AddProcess(CSProcess* prc){
	if(!S_GetProcess(prc->GetPid())){
		sys_echo("[CSERV] Added process %d with canvas %d %d %d %d\n",prc->GetPid(),prc->GetCanvasX(),prc->GetCanvasY(),prc->GetCanvasWidth(),prc->GetCanvasHeight());
		processes.push_back(prc);
	}
}


		
std::vector<CSProcess*> CServer::S_GetAllProcesses(){
	return processes;
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
	rewind(server_pipe);
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
	rewind(sock);
	free(packet);
	return 0;
}

CSContext* CServer::C_GetContext(){
	return CServer::c_ctx;
}

static void cserver_atexit_handlr(){
	sys_echo("[CSRV] Sending CS_TYPE_TERMINATE\n");
}

void CServer::RefreshScreen(){
	fb_optbuff_size(1024,768);
	fb_inject_buffer(c_ctx->GetX(),c_ctx->GetY(),c_ctx->GetSX(),c_ctx->GetSY(),c_ctx->ToPlain(),(uint32_t*)SHARED_MEMORY_START);
}

int CServer::C_InitClient(int sx,int sy){
	if(CServer::Init("/dev/cserver")){
		return 1;
	}
	CSPacket* pack = CSPacket::CreatePacket(CS_TYPE_PROCESS);
	((pid_t*)pack->GetBuffer())[0] = getpid();
	((pid_t*)pack->GetBuffer())[1] = sx;
	((pid_t*)pack->GetBuffer())[2] = sy;
	CServer::C_SendPacket(pack);
	std::atexit(cserver_atexit_handlr);
	sys_echo("[CSRV] Client initialized\n");
	sys_shmem_create(sx*sy*4);
	CServer::c_ctx = new CSContext(sx,sy);
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

uint8_t CSPacket::GetFlags(){
	return this->flags;
}

void CSPacket::SetFlags(uint8_t nw){
	this->flags = nw;
}

CSProcess::CSProcess(pid_t pid){
	this->pid = pid;
	this->packet_filter = [](CSPacket* p){return false;};
}

CSProcess* CSProcess::CreateProcess(pid_t p,uint32_t x,uint32_t y,uint32_t sx,uint32_t sy){
	CSProcess* prc = new CSProcess(p);
	prc->SetCanvasProperties(x,y,sx,sy);
	return prc;
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
uint32_t CSProcess::GetCanvasX(){
		return pos_x;	
}
uint32_t CSProcess::GetCanvasY(){
		return pos_y;
}
uint32_t CSProcess::GetCanvasWidth(){
		return canvas_size_x;
}
uint32_t CSProcess::GetCanvasHeight(){
		return canvas_size_y;
}
void CSProcess::SetCanvasProperties(int x, int y, int sx, int sy){
	this->pos_x = x;
	this->pos_y = y;
	this->canvas_size_x = sx;
	this->canvas_size_y = sy;
}
