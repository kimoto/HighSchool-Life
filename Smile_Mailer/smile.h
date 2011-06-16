/*
 *	smile.c�Ŏg���֐���}�N���̒�`
 *	copyright(c) 2004 kimoto
 */

/***************************************************
				�}�N���̒�`
****************************************************/
#ifdef _DEBUG
#define DebugPrint(message) MessageBox(NULL,message,"DEBUG",MB_OK);
#else
#define DebugPrint(message)
#endif

#define MAILER_NAME "SMILE MAILER"
#define INI_FILE_NAME "config.ini"
#define MAX_BUFFER_SIZE 256
#define WM_SENDMAIL (WM_USER + 100)

/*****************************************************
			�֐��̃v���g�^�C�v�̐錾
******************************************************/
DWORD WINAPI SendMailThread(LPVOID lpParam);
LRESULT CALLBACK MainDlgProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);

BOOL InitApplication(HWND hDlg);
BOOL InitIniFilePath(char *inipath);

BOOL ExitApplication(HWND hDlg);

BOOL GetMXAndSendMail(HWND hDlg,MAILDATA *mdat,BOOL attach_exist);
BOOL GetAttachFilePath(HWND hDlg);

BOOL CheckInputData(HWND hDlg);
BOOL IsEmpty(char *buffer);
BOOL IsFoulMailAddr(char *address);
BOOL IsFoulIPAddr(char *ip_addr);

/******************************************************
			�O���[�o���ϐ��̐錾
*******************************************************/
char ini_filepath[MAX_PATH]="";
