#include "CSocket.h"

#define RELEASE(x) if(x != NULL){ free(x); x = NULL; }

CSocket::CSocket()
{
	Open();
	return;
}

CSocket::~CSocket()
{
	Close();
	return;
}

bool CSocket::Open()
{
	WSADATA wsadata;
	if(WSAStartup(MAKEWORD(2,1),&wsadata) != 0){
		return false;
	}

	m_sock = socket(AF_INET,SOCK_STREAM,0);
	if(m_sock == INVALID_SOCKET){
		return false;
	}

	return true;
}

bool CSocket::Close()
{
	shutdown(m_sock,SD_BOTH);
	closesocket(m_sock);
	WSACleanup();
	return true;
}

// 127.0.0.1
bool CSocket::Connect(char *addr,int port)
{
	SOCKADDR_IN sockaddr_in;
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(port);
	sockaddr_in.sin_addr.S_un.S_addr = inet_addr(addr);

	if(connect(m_sock,(SOCKADDR *)&sockaddr_in,sizeof(sockaddr_in)) != 0){
		return false;
	}
	return true;
}

bool CSocket::Bind(int port)
{
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	if( bind(m_sock,(SOCKADDR *)&addr,sizeof(addr)) < 0){
		return false;
	}

	return true;
}

bool CSocket::Listen(int num)
{
	if( listen(m_sock,num) < 0 ){
		return false;
	}

	return true;
}

bool CSocket::Accept(void)
{
	SOCKADDR_IN sockaddr_in;
	int len = sizeof(sockaddr_in);

	m_sock = accept(m_sock,(SOCKADDR *)&sockaddr_in,&len);
	if(m_sock == INVALID_SOCKET){
		return false;
	}

	return true;
}

/*
bool CSocket::Send(char *buf)
{
	if( send(m_sock,buf,strlen(buf),0) < 0){
		return false;
	}
	return true;
}
*/
/*
bool CSocket::Recv(void)
{
	char temp[5];
	char *buf = NULL;
	int total = 0;
	int size = 0;

	bool once = true;

	RELEASE(m_buffer);

	while(1){
		memset(temp,'\0',sizeof(temp));
		size = recv(m_sock,temp,sizeof(temp) - 1,0);
		if(size < 0){
			return false;
		}
		total += size;

		buf = (char *)malloc(total + 1);
		if(once){
			once = false;

			strcpy(buf,temp);
		}else{
			strcpy(buf,m_buffer);
			strcat(buf,temp);

			free(m_buffer);
			m_buffer = NULL;
		}

		m_buffer = (char *)malloc(total + 1);
		strcpy(m_buffer,buf);

		free(buf);

		if(size != sizeof(temp) - 1){
			break;
		}
	}
	return true;
}
*/

bool CSocket::Send(char *buf,size_t buf_size)
{
	if( send(m_sock,buf,buf_size,0) < 0){
		return false;
	}
	return true;
}

bool CSocket::Recv(char *buf,size_t buf_size)
{
	memset(buf,'\0',buf_size);

	if( recv(m_sock,buf,buf_size - 1,0) < 0 ){
		return false;
	}

	buf[buf_size] = '\0';
	return true;
}

/*
const char *CSocket::GetBuffer(void)
{
	return m_buffer;
}*/