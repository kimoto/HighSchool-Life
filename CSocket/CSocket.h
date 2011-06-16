#include <winsock2.h>
#pragma comment(lib,"wsock32.lib")

class CSocket
{
private:
	char *m_buffer;

public:
	SOCKET m_sock;

	CSocket();
	~CSocket();

	bool Open();
	bool Close();

	bool Connect(char *,int);
	bool Bind(int);
	bool Listen(int);
	bool Accept(void);

/*	bool Send(char *);
	bool Recv(void);*/

	bool Send(char *,size_t);
	bool Recv(char *,size_t);

	//const char* GetBuffer(void);
};