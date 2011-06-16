#include <stdio.h>
#include <stdlib.h>

#include "../CSocket.h"

#define DebugPrint(msg) MessageBox(NULL,msg,"Error",MB_OK)

int main(void)
{
	CSocket sock;

	// connect
	if( !sock.Connect("127.0.0.1",1000) ){
		DebugPrint("Connect Error");
		return -1;
	}

	char buf[256];
	sock.Recv(buf,sizeof(buf));
	puts(buf);
	return 0;
}
