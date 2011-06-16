/*
 *	添付ファイル関連の関数
 *	2004/07/17 : program by kimoto
 */
#include <string.h>
#include <windows.h>
#include "smtp.h"
#include "attach.h"
#include "resource.h"

//#define DEBUG_MODE

#ifdef DEBUG_MODE
#define DebugPrint(message) MessageBox(NULL,message,"DEBUG",MB_OK)
#else
#define DebugPrint(message)
#endif

static int split_file_name(char *filepath,char *filename);
static size_t get_file_size(char *filepath);
static BOOL read_to_buffer(char *filepath,char *buffer,size_t buffer_size);


/*
 *	添付ファイルを設定する
 */
BOOL GetAttachFilePath(HWND hDlg)
{
	OPENFILENAME ofn;
	char full_path[MAX_PATH]="";
	char file_name[MAX_PATH]="";

	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFilter = "すべてのファイル\0*.*\0\0";
	ofn.lpstrFile = full_path;
	ofn.nMaxFile = sizeof(full_path);
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrFileTitle = file_name;
	ofn.nMaxFileTitle = sizeof(file_name);
	GetOpenFileName(&ofn);

	SetDlgItemText(hDlg,IDC_ATTACHFILENAME,full_path);

	return TRUE;
}

/*
 *	指定したファイルが存在するか調べる関数
 *	存在すれば、TRUE,
 *	なければ、FALSE
 */
extern BOOL CheckFileExist(char *filepath)
{
	HANDLE hFile;

	hFile = CreateFile(filepath,GENERIC_READ,FILE_SHARE_READ,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);
	return TRUE;
}


/*
 *	添付ファイルを指定したバッファに読み込む
 */
extern BOOL ReadAttachFile(char *filepath,MAILDATA *mdat)
{
	split_file_name(filepath,mdat->attach_name);
	
	/*ファイルのサイズを取得*/
	mdat->attach_size = get_file_size(filepath);

	/*読み込み*/
	mdat->attach = (char *)GlobalAlloc(GMEM_FIXED,mdat->attach_size + 1);
	read_to_buffer(filepath,mdat->attach,mdat->attach_size);

	return TRUE;
}


/*
 *	ファイルのフルパス名からファイル名を抽出する
 */
static int split_file_name(char *filepath,char *filename)
{
	int len;
	len = strlen(filepath);

	while(len-- > 0){
		if(filepath[len] == '\\'){
			strcpy(filename,&filepath[len + 1]);
			break;
		}
	}
	return 0;
}


/*
 *	指定されたファイル指定したサイズ分、バッファに読み込む
 */
static BOOL read_to_buffer(char *filepath,char *buffer,size_t buffer_size)
{
	HANDLE hFile;
	DWORD dwReadSize;

	hFile = CreateFile(filepath,GENERIC_READ,FILE_SHARE_READ,
		NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		DebugPrint("ファイルの読み込みに失敗しました。");
		CloseHandle(hFile);
		return FALSE;
	}
	ReadFile(hFile,buffer,buffer_size,&dwReadSize,NULL);

	CloseHandle(hFile);
	return TRUE;
}

/*
 *	ファイルサイズを取得する関数
 */
static size_t get_file_size(char *filepath)
{
	HANDLE hFile;
	size_t file_size;

	hFile = CreateFile(filepath,GENERIC_READ,FILE_SHARE_READ,
		NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		CloseHandle(hFile);
		return -1;
	}
	file_size = GetFileSize(hFile,NULL);
	CloseHandle(hFile);
	return file_size;
}
