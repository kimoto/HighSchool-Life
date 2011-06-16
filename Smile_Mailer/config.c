/*
 *	ini�t�@�C������֘A�̊֐�
 */
#include <windows.h>

//#define DEBUG_MODE

#ifdef DEBUG_MODE
#define DebugPrint(message) MessageBox(NULL,message,"DEBUG",MB_OK);
#else
#define DebugPrint(message)
#endif

/*
 *	�ݒ��ǂݍ���ŁA�w�肵���E�B���h�E�ɔ��f������
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
 *	�E�B���h�E�̏���ǂݍ���ŁA�ݒ�t�@�C���ɔ��f������
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
