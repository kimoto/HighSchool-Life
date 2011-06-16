#include <stdio.h>
#include "../CSocket.h"

#define DebugPrint(msg) MessageBox(NULL,msg,"Error",MB_OK)

int main(void)
{
	CSocket *sock = new CSocket;

	puts("Trojan Hource Server\n");
 
	// set server
	if( !sock->Bind(1000) ){
		DebugPrint("Bind Error");
		return 0;
	}
	if( !sock->Listen(5) ){
		DebugPrint("Listen Error");
		return 0;
	}
	if( !sock->Accept() ){
		DebugPrint("Accept Error");
		return 0;
	}

	char buf[] = "This is test.";
	sock->Send(buf,strlen(buf));
	return 0;
}