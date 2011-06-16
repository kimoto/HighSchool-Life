/********************************************************************************************
								Smile-Mailer ver1.0.0
【概要】
Smile-Mailerは、プロバイダのSMTPサーバーを利用せずにメールを送信するツールです。
次のような利点があります。

１．プロバイダのSMTPサーバーが混んでいる時に高速なメール送信を期待できます。
２．相手のメールアドレスだけを知っていればいいのでお手軽簡単設定。
３．相手にメールが届いたか、送信結果が直ちにわかる。

【特徴】
１．件名、本文、送信者名、すべて日本語に対応しています。(JIS)
２．添付ファイルの送信が可能。
３．MFC等を使っていないためプログラムのサイズが小さく、高速。
４．スレッドを使用しているので、送信している間もスムーズな作業が可能。
５．VisualStyleに対応している。
								
																			2004/07/31
																			program by kimoto
*********************************************************************************************/
#include <windows.h>
#include <windowsx.h>
#include "smtp.h"
#include "smile.h"
#include "mx.h"
#include "attach.h"
#include "split.h"
#include "strtable.h"
#include "config.h"
#include "resource.h"


/*
 *	メイン関数
 */
BOOL WINAPI WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nShowCmd)
{
	DialogBox(hInstance,MAKEINTRESOURCE(IDD_MAIN),
		NULL,(DLGPROC)MainDlgProc);

	return TRUE;
}


/*
 *	メインのダイアログプロシージャ
 */
LRESULT CALLBACK MainDlgProc(HWND hDlg,
						 UINT msg,
						 WPARAM wParam,
						 LPARAM lParam)
{
	static HANDLE hThread;
	static DWORD dwID;

	switch(msg){
	case WM_SENDMAIL:
		/*SendMailThreadからのメッセージがここのlParamに来る*/
		SetDlgItemText(hDlg,IDC_RESULT,(char *)lParam);
		return TRUE;
	case WM_INITDIALOG:
		InitApplication(hDlg);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDATTACH:
			GetAttachFilePath(hDlg);
			return TRUE;
		case IDSEND:
			/*メール送信用のスレッドの作成*/
			hThread = CreateThread(NULL,0,SendMailThread,(LPVOID)hDlg,0,&dwID);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg,0);
			return TRUE;
		}
		return TRUE;
	case WM_DESTROY:
		/*スレッドを破棄する*/
		CloseHandle(hThread);
		ExitApplication(hDlg);
		EndDialog(hDlg,0);
		return TRUE;
	case WM_CLOSE:
		SendMessage(hDlg,WM_DESTROY,0,0);
		return TRUE;
	}
	return FALSE;
}


/*
 *	このプログラムの初期化処理
 *	主に、設定ファイルからの読み込みと反映
 */
BOOL InitApplication(HWND hDlg)
{
	HWND hCombo;
	char buffer[256]="";

	/*設定ファイルの保存場所の設定*/
	InitIniFilePath(ini_filepath);

	/*基本的な設定の読み込み、設定*/
	LoadConfig(GetDlgItem(hDlg,IDC_MAILFROM),"MAIL","MAIL FROM",ini_filepath);
	LoadConfig(GetDlgItem(hDlg,IDC_MAILTO),"MAIL","MAIL TO",ini_filepath);
	LoadConfig(GetDlgItem(hDlg,IDC_SENDER),"MAIL","FROM",ini_filepath);
	LoadConfig(GetDlgItem(hDlg,IDC_DNSSERVER),"OPTION","DNS SERVER",ini_filepath);
	LoadConfig(GetDlgItem(hDlg,IDC_PCNAME),"OPTION","PC NAME",ini_filepath);

	/*メールの優先度の初期化*/
	hCombo = GetDlgItem(hDlg,IDC_MAILPRIORITY);
	ComboBox_AddString(hCombo,"1 (Highest)");
	ComboBox_AddString(hCombo,"2 (High)");
	ComboBox_AddString(hCombo,"3 (Normal)");
	ComboBox_AddString(hCombo,"4 (Low)");
	ComboBox_AddString(hCombo,"5 (Lowest)");
	
	/*優先度の3,Normalを初期で選択する*/
	ComboBox_SetCurSel(hCombo,2);

	/*結果を表示するスタティックテキストの初期化*/
	StringTableSetWindowText(GetDlgItem(hDlg,IDC_RESULT),IDS_READY);

	return TRUE;
}


/*
 *	iniファイルの保存場所の設定
 *	プログラムのカレントディレクトリ
 */
BOOL InitIniFilePath(char *inipath)
{
	char filepath[MAX_PATH]="";
	char directory[MAX_PATH]="";

	GetModuleFileName(NULL,filepath,sizeof(filepath));
	SplitDirectoryName(filepath,directory);
	//MessageBox(NULL,directory,"",MB_OK);

	wsprintf(inipath,"%s\\%s",directory,INI_FILE_NAME);
	//MessageBox(NULL,inipath,"",MB_OK);
	return TRUE;
}

/*
 *	このプログラムの終了処理
 */
BOOL ExitApplication(HWND hDlg)
{
	char filepath[1024]="./config.ini";

	SaveConfig(GetDlgItem(hDlg,IDC_MAILFROM),"MAIL","MAIL FROM",filepath);
	SaveConfig(GetDlgItem(hDlg,IDC_MAILTO),"MAIL","MAIL TO",filepath);
	SaveConfig(GetDlgItem(hDlg,IDC_SENDER),"MAIL","FROM",filepath);
	SaveConfig(GetDlgItem(hDlg,IDC_DNSSERVER),"OPTION","DNS SERVER",filepath);
	SaveConfig(GetDlgItem(hDlg,IDC_PCNAME),"OPTION","PC NAME",filepath);	/*2004/07/22:add*/

	return TRUE;
}


/*
 *	メール送信スレッド
 */
DWORD WINAPI SendMailThread(LPVOID lpParam)
{
	MAILDATA mdat;
	HWND hDlg;
	char attach_filepath[MAX_PATH];
	BOOL attach_exist;
	int mailbody_len;			/*本文の長さ*/
	int subject_len;			/*件名の長さ*/

	hDlg = (HWND)lpParam;

	if(CheckInputData(hDlg) != TRUE){
		return 0;
	}

	InitMailData(&mdat);

	GetDlgItemText(hDlg,IDC_SENDER,mdat.sender,sizeof(mdat.sender));
	GetDlgItemText(hDlg,IDC_MAILFROM,mdat.from,sizeof(mdat.from));
	GetDlgItemText(hDlg,IDC_MAILTO,mdat.to,sizeof(mdat.to));
	GetDlgItemText(hDlg,IDC_MAILPRIORITY,mdat.priority,sizeof(mdat.priority));
	GetDlgItemText(hDlg,IDC_PCNAME,mdat.pc_name,sizeof(mdat.pc_name));
	strcpy(mdat.mailer,"SMILE MAILER");
	strcpy(mdat.reply,mdat.from);

	/*添付ファイルを読み込む、*/
	GetDlgItemText(hDlg,IDC_ATTACHFILENAME,attach_filepath,sizeof(attach_filepath));
	attach_exist = CheckFileExist(attach_filepath);
	if(attach_exist == TRUE){
		/*関数内部でメモリを割り当てるので、必ず後で解放すること。*/
		if(ReadAttachFile(attach_filepath,&mdat) == FALSE){
			StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
					IDS_ERROR_READ_ATTACH_FILE,MB_ICONEXCLAMATION);
		}
	}

	/*本文をダイアログボックスから読み込む、解放を忘れずに*/
	mailbody_len = GetWindowTextLength(GetDlgItem(hDlg,IDC_MAILBODY));
	mdat.body = (char *)GlobalAlloc(GMEM_FIXED,mailbody_len + 1);
	GetDlgItemText(hDlg,IDC_MAILBODY,mdat.body,mailbody_len + 1);

	/*件名をダイアログボックスから読み込む、解放を忘れずに*/
	subject_len = GetWindowTextLength(GetDlgItem(hDlg,IDC_MAILSUBJECT));
	mdat.subject = (char *)GlobalAlloc(GMEM_FIXED,subject_len + 1);
	GetDlgItemText(hDlg,IDC_MAILSUBJECT,mdat.subject,subject_len + 1);

	GetMXAndSendMail(hDlg,&mdat,attach_exist);

	GlobalFree(mdat.attach);
	GlobalFree(mdat.body);
	GlobalFree(mdat.subject);

	return 0;
}

/*
 *	MXレコードを取得して、そのメールサーバーに接続して
 *	メール送信をする
 */
BOOL GetMXAndSendMail(HWND hDlg,MAILDATA *mdat,BOOL attach_exist)
{
	MXRECORD mx[20];
	LPMXRECORD lpmx;
	char domain[256]="";
	char dns_addr[256]="";

	lpmx = mx;

	/*送信先アドレスからMXレコードを取得*/
	GetDlgItemText(hDlg,IDC_DNSSERVER,dns_addr,sizeof(dns_addr));
	SplitDomainName(mdat->to,domain);
	StringTableSendMessage(hDlg,WM_SENDMAIL,IDS_QUERY_DNS_SERVER);
	Sleep(300);		/*かっこよく見せるため、抜かしても構わない。*/
	
	if(GetMXRecord(domain,dns_addr,lpmx) != GETMX_ERROR_SUCCESS){
		StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
			IDS_ERROR_DNS_SERVER,MB_ICONEXCLAMATION);
		return FALSE;
	}

	/*MXレコードを頼りに1つずつ順番にメール送信、成功したらループを抜ける*/
	while(lpmx->flag != FALSE){
		StringTableSendMessage(hDlg,WM_SENDMAIL,IDS_SENDING_MAIL);

		if(SendMail(lpmx->domain,mdat,attach_exist,"===SMILE===")
							== SENDMAIL_ERROR_SUCCESS){
			StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
				IDS_SUCCESS_SENDMAIL,MB_OK);
			return TRUE;
		}
		lpmx++;
	}
	/*ここに処理がくるということは、失敗した時*/
	StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
		IDS_ERROR_SENDMAIL,MB_ICONEXCLAMATION);

	return FALSE;
}

/*
 *	ダイアログボックスに入力されたデータが正しいものか調べる関数
 */
BOOL CheckInputData(HWND hDlg)
{
	char buffer[MAX_BUFFER_SIZE]="";

	/*メール発信元*/
	//GetDlgItemText(hDlg,IDC_MAILFROM,buffer,sizeof(buffer));
	/*if(IsEmpty(buffer)){
		StringTableSendMessage(hDlg,WM_SENDMAIL,IDS_ERROR_MAILFROM_EMPTY);
		return FALSE;
	}*/
	/*メール発信先*/
	GetDlgItemText(hDlg,IDC_MAILTO,buffer,sizeof(buffer));
	if(IsEmpty(buffer)){
		StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
			IDS_ERROR_MAILTO_EMPTY,MB_ICONEXCLAMATION);
		return FALSE;
	}
	if(IsFoulMailAddr(buffer)){
		StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
			IDS_ERROR_MAILTO_FOUL,MB_ICONEXCLAMATION);
		return FALSE;
	}
	/*DNSサーバーのIPアドレス*/
	GetDlgItemText(hDlg,IDC_DNSSERVER,buffer,sizeof(buffer));
	if(IsEmpty(buffer)){
		StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
			IDS_ERROR_DNS_SERVER_EMPTY,MB_ICONEXCLAMATION);
		return FALSE;
	}
	if(IsFoulIPAddr(buffer)){
		StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
			IDS_ERROR_DNS_SERVER_FOUL,MB_ICONEXCLAMATION);
		return FALSE;
	}
	return TRUE;
}

/*
 *	指定したバッファが、空だったときTRUE
 */
BOOL IsEmpty(char *buffer)
{
	if(strcmp(buffer,"") != 0){
		return FALSE;
	}
	return TRUE;
}

/*
 *	指定したアドレスが不正な形式だったらTRUE
 */
BOOL IsFoulMailAddr(char *address)
{
	char *strptr;

	/*@のあとに.が１つ含まれているか調べるだけの単純なアルゴリズム*/
	/*バグはないと思う*/
	strptr = strstr(address,"@");
	if(strptr == NULL){
		return TRUE;
	}
	strptr++;

	strptr = strstr(strptr,".");
	if(strptr == NULL){
		return TRUE;
	}

	return FALSE;
}

/*
 *	指定したアドレスが、IPアドレス(IPv4)形式か調べて、
 *	不正な形式だったらTRUEを返す
 *	(192.168.0.1) => 正しいからFALSE
 */
BOOL IsFoulIPAddr(char *ip_addr)
{
	char *strptr;

	/*.が３つ以上あるかどうか調べるだけの単純なアルゴリズム*/
	strptr = strstr(ip_addr,".");
	if(strptr == NULL){
		return TRUE;
	}
	strptr++;
	
	strptr = strstr(strptr,".");
	if(strptr == NULL){
		return TRUE;
	}
	strptr++;
	
	strptr = strstr(strptr,".");
	if(strptr == NULL){
		return TRUE;
	}
	return FALSE;
}