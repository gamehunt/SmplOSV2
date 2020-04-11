#include <cserv/cserv.h>
#include <unistd.h>

int main(){
	CServer::C_InitClient();
	CSPacket* p = new CSPacket(CS_TYPE_WIDGET);
	((uint32_t*)p->GetBuffer())[0] = getpid();
	((uint32_t*)p->GetBuffer())[1] = 0;
	CServer::C_SendPacket(p);
	while(1);
	return 0;
}
