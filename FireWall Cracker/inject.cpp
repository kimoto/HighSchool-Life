/************************************************************************************
						DLLのインジェクションを簡単に行うためのライブラリ
						
						2004/08/08	C++仕様に合わせた。
									バグ発見、解消。
									Windows用にいろいろとデータ型を変更

						2004/07/24	ヘッダファイルにきちんと
									関数が宣言できてなかった（汗）
						2004/07/22 by kimoto
*************************************************************************************/
#include <windows.h>
#include <tlhelp32.h>

#include "inject.h"

//#define DEBUG_MODE
#ifdef DEBUG_MODE
#define DebugPrint(message) MessageBox(NULL,message,"DEBUG",MB_OK);
#else
#define DebugPrint(message)
#endif

static DWORD GetProcessIDNumber(TCHAR *cProcessName);
static PWSTR WriteTargetMemory(HANDLE hProcess,TCHAR *data,UINT data_len);
static PTHREAD_START_ROUTINE GetLoadLibraryAddress(void);

/**********************************************************************************
						使用例
BOOL WINAPI WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nShowCmd)
{
	char szLibFilePath[MAX_PATH];
	GetModuleFileName(NULL,szLibFilePath,sizeof(szLibFilePath));
	
	_tcscpy(_tcsrchr(szLibFilePath,'\\') + 1,"Chaos.dll");

	//DLLをプロセスに注入する
	if(InjectToProcess(szLibFilePath,"explorer.exe") == FALSE){
		DebugPrint("InjectToProcess error");
		return FALSE;
	}
	DebugPrint("Success!!");

	return TRUE;
}
**************************************************************************************/

/*
 *	指定したDLLを指定したプロセスに注入する
 *	DLLは、必ず絶対パスで指定すること
 */
extern BOOL InjectToProcess(TCHAR *cDllName,
							TCHAR *cTargetProcess)
{
	DWORD dwProcessID;
	HANDLE hProcess;
	PWSTR RemoteProcessMemory;
	PTHREAD_START_ROUTINE pfnThreadRtn;
	HANDLE hThread;

	/*targetの名前からIDを引き出す*/
	dwProcessID = GetProcessIDNumber(cTargetProcess);
	if(dwProcessID < 0){
		DebugPrint("dwProcessID error");
		return FALSE;
	}
	/*targetのプロセスを開く*/
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE,FALSE,dwProcessID);
	if(hProcess == NULL){
		DebugPrint("OpenProcess error");
		return FALSE;
	}
	/*DLLの名前をメモリに書き込む*/
	RemoteProcessMemory = WriteTargetMemory(hProcess,cDllName,strlen(cDllName));
	if(RemoteProcessMemory == NULL){
		DebugPrint("WriteTargetMemory error");
		return FALSE;
	}
	/*LoadLibraryのアドレスを取得*/
	pfnThreadRtn = GetLoadLibraryAddress();
	if(pfnThreadRtn == NULL){
		DebugPrint("pfnThreadRtn error");
		return FALSE;
	}
	/*targetのプロセスの中でdllを実行する*/
	hThread = CreateRemoteThread(hProcess,NULL,0,pfnThreadRtn,
		RemoteProcessMemory,0,NULL);
	if(hThread == NULL){
		DebugPrint("CrateRemoteThread error");
		return FALSE;
	}
	/*Threadが終わるまで待ち続ける*/
	return TRUE;
}

/*
 *	プロセス名からプロセスIDを求める
 */
static DWORD GetProcessIDNumber(TCHAR *cProcessName)
{
	HANDLE hSnap;
	PROCESSENTRY32 pe;
	DWORD dwProcessID;
	
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(hSnap == INVALID_HANDLE_VALUE){
		DebugPrint("Create error");
		return -1;
	}
	ZeroMemory(&pe,sizeof(pe));
	pe.dwSize = sizeof(pe);

	Process32First(hSnap,&pe);

	do{
		/*見つかった時、そのID番号を返す*/
		if(lstrcmp(pe.szExeFile,cProcessName) == 0){
			dwProcessID = pe.th32ProcessID;
			CloseHandle(hSnap);
			return dwProcessID;
		}
	}while(Process32Next(hSnap,&pe) != FALSE);

	CloseHandle(hSnap);
	return -1;
}

/*
 *	指定したプロセスのメモリ空間に
 *	指定したデータを書き込む関数
 */
static PWSTR WriteTargetMemory(HANDLE hProcess,
							   TCHAR *cData,
							   UINT uSize)
{
	PWSTR RemoteProcessMemory;

	if(hProcess == NULL){
		return NULL;
	}
	/*targetのメモリ空間に新しくメモリを作る*/
	RemoteProcessMemory = (PWSTR)VirtualAllocEx(hProcess,NULL,
		uSize + 1,MEM_COMMIT,PAGE_READWRITE);
	if(RemoteProcessMemory == NULL){
		DebugPrint("WriteTargetMemory error");
		return NULL;
	}
	/*そこに書き込む*/
	if(WriteProcessMemory(hProcess,RemoteProcessMemory,
		cData,uSize,NULL) == 0){
		DebugPrint("WriteProcessMemory error");
		return NULL;
	}
	return RemoteProcessMemory;
}

/*
 *	"LoadLibrary"関数へのポインタを取得する関数
 */
static PTHREAD_START_ROUTINE GetLoadLibraryAddress(void)
{
	HMODULE hKernel;
	PTHREAD_START_ROUTINE pfnThreadRtn;

	hKernel = GetModuleHandle("Kernel32");
	if(hKernel == NULL){
		DebugPrint("kernel error");
		return NULL;
	}
	pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(hKernel,"LoadLibraryA");
	if(pfnThreadRtn == NULL){
		DebugPrint("GetProcAddress error");
		return NULL;
	}
	return pfnThreadRtn;
}