/*
 *	DLLのインジェクションを簡単に行うためのライブラリ
 *	created by kimoto
 */
#ifndef _INJECT_H_INCLUDED_
#define _INJECT_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
	
/*
 *	指定したプロセスにDLLを注入する
 */
extern BOOL InjectToProcess(TCHAR *cDllName,TCHAR *cTarget);

#ifdef __cplusplus
}
#endif

#endif /*_INJECT_H_INCLUDED_*/