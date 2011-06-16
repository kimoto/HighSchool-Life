/************************************************************************************
						DLL�̃C���W�F�N�V�������ȒP�ɍs�����߂̃��C�u����
						
						2004/08/08	C++�d�l�ɍ��킹���B
									�o�O�����A�����B
									Windows�p�ɂ��낢��ƃf�[�^�^��ύX

						2004/07/24	�w�b�_�t�@�C���ɂ������
									�֐����錾�ł��ĂȂ������i���j
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
						�g�p��
BOOL WINAPI WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nShowCmd)
{
	char szLibFilePath[MAX_PATH];
	GetModuleFileName(NULL,szLibFilePath,sizeof(szLibFilePath));
	
	_tcscpy(_tcsrchr(szLibFilePath,'\\') + 1,"Chaos.dll");

	//DLL���v���Z�X�ɒ�������
	if(InjectToProcess(szLibFilePath,"explorer.exe") == FALSE){
		DebugPrint("InjectToProcess error");
		return FALSE;
	}
	DebugPrint("Success!!");

	return TRUE;
}
**************************************************************************************/

/*
 *	�w�肵��DLL���w�肵���v���Z�X�ɒ�������
 *	DLL�́A�K����΃p�X�Ŏw�肷�邱��
 */
extern BOOL InjectToProcess(TCHAR *cDllName,
							TCHAR *cTargetProcess)
{
	DWORD dwProcessID;
	HANDLE hProcess;
	PWSTR RemoteProcessMemory;
	PTHREAD_START_ROUTINE pfnThreadRtn;
	HANDLE hThread;

	/*target�̖��O����ID�������o��*/
	dwProcessID = GetProcessIDNumber(cTargetProcess);
	if(dwProcessID < 0){
		DebugPrint("dwProcessID error");
		return FALSE;
	}
	/*target�̃v���Z�X���J��*/
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE,FALSE,dwProcessID);
	if(hProcess == NULL){
		DebugPrint("OpenProcess error");
		return FALSE;
	}
	/*DLL�̖��O���������ɏ�������*/
	RemoteProcessMemory = WriteTargetMemory(hProcess,cDllName,strlen(cDllName));
	if(RemoteProcessMemory == NULL){
		DebugPrint("WriteTargetMemory error");
		return FALSE;
	}
	/*LoadLibrary�̃A�h���X���擾*/
	pfnThreadRtn = GetLoadLibraryAddress();
	if(pfnThreadRtn == NULL){
		DebugPrint("pfnThreadRtn error");
		return FALSE;
	}
	/*target�̃v���Z�X�̒���dll�����s����*/
	hThread = CreateRemoteThread(hProcess,NULL,0,pfnThreadRtn,
		RemoteProcessMemory,0,NULL);
	if(hThread == NULL){
		DebugPrint("CrateRemoteThread error");
		return FALSE;
	}
	/*Thread���I���܂ő҂�������*/
	return TRUE;
}

/*
 *	�v���Z�X������v���Z�XID�����߂�
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
		/*�����������A����ID�ԍ���Ԃ�*/
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
 *	�w�肵���v���Z�X�̃�������Ԃ�
 *	�w�肵���f�[�^���������ފ֐�
 */
static PWSTR WriteTargetMemory(HANDLE hProcess,
							   TCHAR *cData,
							   UINT uSize)
{
	PWSTR RemoteProcessMemory;

	if(hProcess == NULL){
		return NULL;
	}
	/*target�̃�������ԂɐV���������������*/
	RemoteProcessMemory = (PWSTR)VirtualAllocEx(hProcess,NULL,
		uSize + 1,MEM_COMMIT,PAGE_READWRITE);
	if(RemoteProcessMemory == NULL){
		DebugPrint("WriteTargetMemory error");
		return NULL;
	}
	/*�����ɏ�������*/
	if(WriteProcessMemory(hProcess,RemoteProcessMemory,
		cData,uSize,NULL) == 0){
		DebugPrint("WriteProcessMemory error");
		return NULL;
	}
	return RemoteProcessMemory;
}

/*
 *	"LoadLibrary"�֐��ւ̃|�C���^���擾����֐�
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