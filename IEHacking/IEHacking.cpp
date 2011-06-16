//======================================================
//		ファイアウォール貫通サンプル
//		2004/08/19	by kimoto
//======================================================
#include <windows.h>
#include <winsock.h>
#pragma comment(lib,"wsock32.lib")

#include "resource.h"

//======================================================
//		マクロの定義
//======================================================
#define SMTP_PORT_NUMBER 25

#define MSGBOX(msg) MessageBox(NULL,msg,"Check",MB_OK);

//======================================================
//		関数のプロトタイプの宣言
//======================================================
BOOL SendMail(HWND);
int vsend(SOCKET sock,char *format,...);
LRESULT CALLBACK DlgProc(HWND,UINT,WPARAM,LPARAM);

//======================================================
//		DLLのメイン関数(エントリポイント）
//======================================================
BOOL WINAPI DllMain(HANDLE hModule,
					DWORD dwReason,
					LPVOID)
{
	switch(dwReason){
	
	//DLLが初期化されるときに呼ばれる
	case DLL_PROCESS_ATTACH:

		//設定ダイアログを表示する
		DialogBox((HINSTANCE)hModule,
			MAKEINTRESOURCE(IDD_MAIN),
			NULL,(DLGPROC)DlgProc);

		return TRUE;
	}

	return TRUE;
}


//===============================================================
//		メインのダイアログプロシージャ
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
	
			//ここに送信するモジュールを書く
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
//		メール送信モジュール
//		一応、クラスを理解していない人は多いので
//		汎用的な、再利用しやすいC言語ライクなモジュールにした
//		(この辺は、クラスにしたほうが楽です。)
//
//		きちんとした例外処理をしていないので注意が必要
//================================================================
BOOL SendMail(HWND hWnd)
{
	MessageBox(
		NULL,
		"DLLのなかのSendMail関数が呼ばれました。\n"
		"メール送信を開始します。\n"
		"ついでに言っておきますが、このプログラムは\n"
		"WindowsNT系専用になります。(CreateRemoteThread関数を使用)",
		"確認",
		MB_OK);

	WSADATA wsadata;
	WSAStartup(MAKEWORD(1,1),&wsadata);

	//ソケット作成
	SOCKET sock;
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock == INVALID_SOCKET){
		return FALSE;
	}

	//指定したSMTPサーバーに接続する
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

	//実際のSMTPとのやり取り
	char buffer[1024];		//受信用のバッファ

	//最初の応答がSMTPからやってくる
	//この辺のエラー処理もメンドクサイからキャンセル
	recv(sock,buffer,sizeof(buffer),0);
	MSGBOX(buffer);

	//HELO コマンド
	vsend(sock,"HELO localhost\r\n");
	recv(sock,buffer,sizeof(buffer),0);
	MSGBOX(buffer);

	//MAIL FROM コマンド - いろいろ変えても大丈夫
	//送信元メールアドレスを入力してください。
	TCHAR szMailFrom[256];
	GetDlgItemText(hWnd,IDC_MAILFROM,szMailFrom,sizeof(szMailFrom));

	vsend(sock,"MAIL FROM:<%s>\r\n",szMailFrom);
	recv(sock,buffer,sizeof(buffer),0);
	MSGBOX(buffer);

	//RCPT TO コマンド - ここも変えちゃう
	//送信先メールアドレスを入力してください。
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

	//QUIT コマンド
	vsend(sock,"QUIT\r\n");
	recv(sock,buffer,sizeof(buffer),0);
	MSGBOX(buffer);

	//送信完了を表示する
	MSGBOX("メール送信完了しました。\n"
		"メールがきちんと届いているか確認してください。");

	//接続を閉じる
	closesocket(sock);
	WSACleanup();

	return TRUE;
}


//=======================================================
//		可変個引数のsend関数
//		戻り値は、send関数の戻り値
//		1024文字まで対応、文字列しか送信できない。
//		バイナリは無理。
//=======================================================
int vsend(SOCKET sock,char *format,...)
{
	int result;
	char send_buf[1024]="";
	va_list args;	//可変個引数集
	
	va_start(args,format);
	wvsprintf(send_buf,format,args);
	MSGBOX(send_buf);
	
	result = send(sock,send_buf,strlen(send_buf),0);
	
	va_end(args);
	
	return result;
}
