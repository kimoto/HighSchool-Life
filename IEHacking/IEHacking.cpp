//======================================================
//		�t�@�C�A�E�H�[���ђʃT���v��
//		2004/08/19	by kimoto
//======================================================
#include <windows.h>
#include <winsock.h>
#pragma comment(lib,"wsock32.lib")

#include "resource.h"

//======================================================
//		�}�N���̒�`
//======================================================
#define SMTP_PORT_NUMBER 25

#define MSGBOX(msg) MessageBox(NULL,msg,"Check",MB_OK);

//======================================================
//		�֐��̃v���g�^�C�v�̐錾
//======================================================
BOOL SendMail(HWND);
int vsend(SOCKET sock,char *format,...);
LRESULT CALLBACK DlgProc(HWND,UINT,WPARAM,LPARAM);

//======================================================
//		DLL�̃��C���֐�(�G���g���|�C���g�j
//======================================================
BOOL WINAPI DllMain(HANDLE hModule,
					DWORD dwReason,
					LPVOID)
{
	switch(dwReason){
	
	//DLL�������������Ƃ��ɌĂ΂��
	case DLL_PROCESS_ATTACH:

		//�ݒ�_�C�A���O��\������
		DialogBox((HINSTANCE)hModule,
			MAKEINTRESOURCE(IDD_MAIN),
			NULL,(DLGPROC)DlgProc);

		return TRUE;
	}

	return TRUE;
}


//===============================================================
//		���C���̃_�C�A���O�v���V�[�W��
//===============================================================
LRESULT CALLBACK DlgProc(HWND hWnd,
						 UINT msg,
						 WPARAM wParam,
						 LPARAM lParam)
{
	switch(msg){
	case WM_COMMAND:
		switch(wParam){
		case IDOK:
	
			//�����ɑ��M���郂�W���[��������
			SendMail(hWnd);

			return 0;

		case IDCANCEL:
			EndDialog(hWnd,0);
			return 0;
		}
		return 0;

	case WM_DESTROY:
		return 0;
	}
	return 0;
}

//================================================================
//		���[�����M���W���[��
//		�ꉞ�A�N���X�𗝉����Ă��Ȃ��l�͑����̂�
//		�ėp�I�ȁA�ė��p���₷��C���ꃉ�C�N�ȃ��W���[���ɂ���
//		(���̕ӂ́A�N���X�ɂ����ق����y�ł��B)
//
//		������Ƃ�����O���������Ă��Ȃ��̂Œ��ӂ��K�v
//================================================================
BOOL SendMail(HWND hWnd)
{
	MessageBox(
		NULL,
		"DLL�̂Ȃ���SendMail�֐����Ă΂�܂����B\n"
		"���[�����M���J�n���܂��B\n"
		"���łɌ����Ă����܂����A���̃v���O������\n"
		"WindowsNT�n��p�ɂȂ�܂��B(CreateRemoteThread�֐����g�p)",
		"�m�F",
		MB_OK);

	WSADATA wsadata;
	WSAStartup(MAKEWORD(1,1),&wsadata);

	//�\�P�b�g�쐬
	SOCKET sock;
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock == INVALID_SOCKET){
		return FALSE;
	}

	//�w�肵��SMTP�T�[�o�[�ɐڑ�����
	TCHAR szSmtpServer[MAX_PATH];
	GetDlgItemText(hWnd,IDC_SMTPSERVER,szSmtpServer,sizeof(szSmtpServer));

	LPHOSTENT lphost = gethostbyname(szSmtpServer);
	if(lphost == NULL){
		return FALSE;
	}
	SOCKADDR_IN sockaddr_in;
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(SMTP_PORT_NUMBER);
	sockaddr_in.sin_addr = *(LPIN_ADDR)*lphost->h_addr_list;

	if(SOCKET_ERROR == connect(sock,(SOCKADDR *)&sockaddr_in,sizeof(sockaddr_in))){
		return FALSE;
	}

	//���ۂ�SMTP�Ƃ̂����
	char buffer[1024];		//��M�p�̃o�b�t�@

	//�ŏ��̉�����SMTP�������Ă���
	//���̕ӂ̃G���[�����������h�N�T�C����L�����Z��
	recv(sock,buffer,sizeof(buffer),0);
	MSGBOX(buffer);

	//HELO �R�}���h
	vsend(sock,"HELO localhost\r\n");
	recv(sock,buffer,sizeof(buffer),0);
	MSGBOX(buffer);

	//MAIL FROM �R�}���h - ���낢��ς��Ă����v
	//���M�����[���A�h���X����͂��Ă��������B
	TCHAR szMailFrom[256];
	GetDlgItemText(hWnd,IDC_MAILFROM,szMailFrom,sizeof(szMailFrom));

	vsend(sock,"MAIL FROM:<%s>\r\n",szMailFrom);
	recv(sock,buffer,sizeof(buffer),0);
	MSGBOX(buffer);

	//RCPT TO �R�}���h - �������ς����Ⴄ
	//���M�惁�[���A�h���X����͂��Ă��������B
	TCHAR szRcptTo[256];
	GetDlgItemText(hWnd,IDC_RCPTTO,szRcptTo,sizeof(szRcptTo));

	vsend(sock,"RCPT TO:<%s>\r\n",szRcptTo);
	recv(sock,buffer,sizeof(buffer),0);
	MSGBOX(buffer);

	//DATA
	vsend(sock,"DATA\r\n");
	recv(sock,buffer,sizeof(buffer),0);
	MSGBOX(buffer);

	//DATA END
	vsend(sock,"\r\n.\r\n");

	//QUIT �R�}���h
	vsend(sock,"QUIT\r\n");
	recv(sock,buffer,sizeof(buffer),0);
	MSGBOX(buffer);

	//���M������\������
	MSGBOX("���[�����M�������܂����B\n"
		"���[����������Ɠ͂��Ă��邩�m�F���Ă��������B");

	//�ڑ������
	closesocket(sock);
	WSACleanup();

	return TRUE;
}


//=======================================================
//		�ό�����send�֐�
//		�߂�l�́Asend�֐��̖߂�l
//		1024�����܂őΉ��A�����񂵂����M�ł��Ȃ��B
//		�o�C�i���͖����B
//=======================================================
int vsend(SOCKET sock,char *format,...)
{
	int result;
	char send_buf[1024]="";
	va_list args;	//�ό����W
	
	va_start(args,format);
	wvsprintf(send_buf,format,args);
	MSGBOX(send_buf);
	
	result = send(sock,send_buf,strlen(send_buf),0);
	
	va_end(args);
	
	return result;
}
