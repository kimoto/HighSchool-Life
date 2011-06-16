/*
 *	iniファイル操作関連の関数
 *	copyright(c) 2004 kimoto
 */

#ifndef _INI_H_INCLUDED_
#define _INI_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus */

BOOL LoadConfig(HWND hWnd,char *app,char *key,char *filepath);
BOOL SaveConfig(HWND hWnd,char *app,char *key,char *filepath);

#ifdef __cplusplus
}
#endif	/*__cplusplus */

#endif	/* _INI_H_INCLUDED_ */