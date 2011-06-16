/*
 *	iniファイル操作関連の関数
 */
#include <windows.h>

//#define DEBUG_MODE

#ifdef DEBUG_MODE
#define DebugPrint(message) MessageBox(NULL,message,"DEBUG",MB_OK);
#else
#define DebugPrint(message)
#endif

/*
 *	設定を読み込んで、指定したウィンドウに反映させる
 */
extern BOOL LoadConfig(HWND hWnd,
				char *app,
				char *key,
				char *filepath)
{
	char buffer[1024];

	GetPrivateProfileString(app,key,NULL,buffer,
		sizeof(buffer),filepath);
	SetWindowText(hWnd,buffer);
	//SetDlgItemText(hDlg,nIDDlgItem,buffer);

	return TRUE;
}


/*
 *	ウィンドウの情報を読み込んで、設定ファイルに反映させる
 */
extern BOOL SaveConfig(HWND hWnd,
				char *app,
				char *key,
				char *filepath)
{
	char buffer[1024];

	GetWindowText(hWnd,buffer,sizeof(buffer));
	//GetDlgItemText(hDlg,nIDDlgItem,buffer,sizeof(buffer));
	WritePrivateProfileString(app,key,buffer,filepath);
	
	return TRUE;
}
