#include <cserv/cserv.h>
#include <unistd.h>
#include <cstring>

const char* art = "   #@@@@@@@@                            &@@@@@@@&   \n"          
                  "     @@@@@@@@@                        ,@@@@@@@@     \n"          
                  "       @@@@@@@@                      @@@@@@@@,      \n"          
                  "        #@@@@@@@@                  &@@@@@@@&        \n"          
                  "          @@@@@@@@@              *@@@@@@@@          \n"          
                  "            @@@@@@@@            @@@@@@@@,           \n"          
                  "             #@@@@@@@@        &@@@@@@@~             \n"          
                  "               @@@@@@@@(    @@@@@@@@@               \n"          
                  "                .@@@@@@@@. @@@@@@@@,                \n"          
                  "                  #@@@@@@@@@@@@@@~                  \n"          
                  "                    @@@@@@@@@@@@                    \n"          
                  "                    ,@@@@@@@@@*                     \n"          
                  "                   @@@@@@@@@@@@@                    \n"          
                  "                 ~@@@@@@@@@@@@@@@#                  \n"          
                  "               ,@@@@@@@@  .@@@@@@@@.                \n"          
                  "              @@@@@@@@*     #@@@@@@@@               \n"          
                  "            ~@@@@@@@&         @@@@@@@@#             \n"          
                  "          ,@@@@@@@@            .@@@@@@@@.           \n"          
                  "         @@@@@@@@,               ~@@@@@@@@          \n"          
                  "       &@@@@@@@&                   @@@@@@@@(        \n"          
                  "     ,@@@@@@@@                      ,@@@@@@@@       \n"          
                  "    @@@@@@@@,                         ~@@@@@@@@     \n"          
                  "  &@@@@@@@&                             @@@@@@@@*   \n"          
                  ",@@@@@@@@                                ,@@@@@@@@  \n"; 

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
	((uint32_t*)p->GetBuffer())[3] = 0;
	((uint32_t*)p->GetBuffer())[4] = 0;
	CServer::C_SendPacket(p);
	((uint32_t*)p->GetBuffer())[1] = WIDGET_PACK_RES;
	((uint32_t*)p->GetBuffer())[2] = 0;
	((uint32_t*)p->GetBuffer())[3] = 1024;
	((uint32_t*)p->GetBuffer())[4] = 768;
	CServer::C_SendPacket(p);
	int iter = 0;
	while(iter < strlen(art)){
		((uint32_t*)p->GetBuffer())[1] = WIDGET_PACK_UPD;
		((uint32_t*)p->GetBuffer())[2] = 0;
		char* text = reinterpret_cast<char*>(&(((uint32_t*)p->GetBuffer())[3]));
		memset(text,0,128);
		const char* str = &art[iter];
		text[0] = 0;
		std::memcpy(text+1,str,127);
		iter+=127;
		CServer::C_SendPacket(p);
	}
	while(1);
	return 0;
}
