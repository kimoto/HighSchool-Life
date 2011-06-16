/*
 *	���\�[�X��"StringTable"���y�Ɏg�����߂̊֐��Q
 */
#include <windows.h>

#define MAX_BUFFER_SIZE 1024

/*
 *	StringTable����ǂݍ����SendMessage����
 */
extern BOOL StringTableSendMessage(HWND hWnd,UINT msg,UINT uID)
{
	char buffer[MAX_BUFFER_SIZE]="";

	LoadString(NULL,uID,buffer,sizeof(buffer));
	SendMessage(hWnd,msg,0,(LPARAM)buffer);
	return TRUE;
}
/*
 *	StringTable����ǂݍ����SendMessage������ɁA
 *	�w�肵��Beep����炷
 */
extern BOOL StringTableSendMessageBeep(HWND hWnd,UINT msg,UINT uID,UINT uType)
{
	StringTableSendMessage(hWnd,msg,uID);
	MessageBeep(uType);

	return TRUE;
}
/*
 *	StringTable����ǂݍ����MessageBox����
 */
extern BOOL StringTableMsgBox(HWND hWnd,UINT uID,char *caption,UINT uType)
{
	char buffer[MAX_BUFFER_SIZE]="";
	
	LoadString(NULL,uID,buffer,sizeof(buffer));
	MessageBox(hWnd,buffer,caption,uType);

	return TRUE;
}

/*
 *	StringTable����ǂݍ����SetWindowText����
 */
extern BOOL StringTableSetWindowText(HWND hWnd,UINT uID)
{
	char buffer[MAX_BUFFER_SIZE]="";

	LoadString(NULL,uID,buffer,sizeof(buffer));
	SetWindowText(hWnd,buffer);

	return TRUE;
}