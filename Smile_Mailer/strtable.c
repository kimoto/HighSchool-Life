/*
 *	リソースの"StringTable"を楽に使うための関数群
 */
#include <windows.h>

#define MAX_BUFFER_SIZE 1024

/*
 *	StringTableから読み込んでSendMessageする
 */
extern BOOL StringTableSendMessage(HWND hWnd,UINT msg,UINT uID)
{
	char buffer[MAX_BUFFER_SIZE]="";

	LoadString(NULL,uID,buffer,sizeof(buffer));
	SendMessage(hWnd,msg,0,(LPARAM)buffer);
	return TRUE;
}
/*
 *	StringTableから読み込んでSendMessageした後に、
 *	指定したBeep音を鳴らす
 */
extern BOOL StringTableSendMessageBeep(HWND hWnd,UINT msg,UINT uID,UINT uType)
{
	StringTableSendMessage(hWnd,msg,uID);
	MessageBeep(uType);

	return TRUE;
}
/*
 *	StringTableから読み込んでMessageBoxする
 */
extern BOOL StringTableMsgBox(HWND hWnd,UINT uID,char *caption,UINT uType)
{
	char buffer[MAX_BUFFER_SIZE]="";
	
	LoadString(NULL,uID,buffer,sizeof(buffer));
	MessageBox(hWnd,buffer,caption,uType);

	return TRUE;
}

/*
 *	StringTableから読み込んでSetWindowTextする
 */
extern BOOL StringTableSetWindowText(HWND hWnd,UINT uID)
{
	char buffer[MAX_BUFFER_SIZE]="";

	LoadString(NULL,uID,buffer,sizeof(buffer));
	SetWindowText(hWnd,buffer);

	return TRUE;
}