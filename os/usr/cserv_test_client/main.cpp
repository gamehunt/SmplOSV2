#include <cserv/cserv.h>
#include <unistd.h>
#include <cstring>

int main(){
	CServer::C_InitClient();
	
	//TODO I need wrapper for this
	
	CSPacket* p = new CSPacket(CS_TYPE_WIDGET);
	((uint32_t*)p->GetBuffer())[0] = getpid();
	((uint32_t*)p->GetBuffer())[1] = WIDGET_PACK_ADD;
	((uint32_t*)p->GetBuffer())[2] = 1;
	CServer::C_SendPacket(p);
	((uint32_t*)p->GetBuffer())[1] = WIDGET_PACK_MOV;
	((uint32_t*)p->GetBuffer())[2] = 0;
	((uint32_t*)p->GetBuffer())[3] = 300;
	((uint32_t*)p->GetBuffer())[4] = 500;
	CServer::C_SendPacket(p);
	((uint32_t*)p->GetBuffer())[1] = WIDGET_PACK_RES;
	((uint32_t*)p->GetBuffer())[2] = 0;
	((uint32_t*)p->GetBuffer())[3] = 600;
	((uint32_t*)p->GetBuffer())[4] = 600;
	CServer::C_SendPacket(p);
	((uint32_t*)p->GetBuffer())[1] = WIDGET_PACK_UPD;
	((uint32_t*)p->GetBuffer())[2] = 0;
	const char* text = "TEST TEXT SCREEN";
	std::strcpy((char*)(&(((uint32_t*)p->GetBuffer())[3])),text);
	//sys_echo("%s\n",(char*)(((uint32_t*)p->GetBuffer())[3]));
	CServer::C_SendPacket(p);
	while(1);
	return 0;
}
