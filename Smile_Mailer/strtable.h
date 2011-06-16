/*
 *	リソースの"StringTable"を楽に使うための関数群
 */
#ifndef _STRTABLE_H_INCLUDED_
#define _STRTABLE_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus */

/*StringTable	SendMesage*/
extern BOOL StringTableSendMessage(HWND hWnd,UINT msg,UINT uID);
extern BOOL StringTableSendMessageBeep(HWND hWnd,UINT msg,UINT uID,UINT uType);

/*StringTable	MessageBox*/
extern BOOL StringTableMsgBox(HWND hWnd,UINT uID,char *caption,UINT uType);
/*StringTable	SetWindowText*/
extern BOOL StringTableSetWindowText(HWND hWnd,UINT uID);

#ifdef __cplusplus
}
#endif	/*__cplusplus */

#endif	/* _STRTABLE_H_INCLUDED_ */
