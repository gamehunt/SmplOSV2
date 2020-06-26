#include <cserv/cserv.h>
#include <unistd.h>
#include <cstring>
#include <fb.h>
#include <cserv/widgets/debug.h>

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
	CServer::C_InitClient(100,100);
	fb_init("",100,100,0);
	bool reverse = false;
	while(1){
		CServer::C_GetContext()->clear();
		uint32_t* plain = CServer::C_GetContext()->ToPlain();
		fb_rect(0,0,20,20,0x00AABBCC,true,plain);
		CServer::C_GetContext()->FromPlain(plain);
		CServer::RefreshScreen();
		sys_yield();
	}
	return 0;
}
