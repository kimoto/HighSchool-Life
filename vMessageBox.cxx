#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// win環境じゃないんで適当に
typedef int* HWND;
typedef char* LPSTR;
typedef unsigned int UINT;
#define MB_OK		0
#define MB_CANCEL	-1

#define MessageBox(a,b,c,d) puts(b);

// 可変個引数メッセージボックス
// 戻り値は、メッセージボックスのをそのまま返す
int vMessageBox(
	HWND hWnd,
	LPSTR lpCaption,
	UINT uType,
	char *format,
	...)
{
	// define
	va_list args;
	int len = 0;
	int result = 0;
	char *buffer = NULL;

	va_start(args,format);

	// create buffer
	len = snprintf(NULL,0,format,args);
	buffer = new char[len + 1];
	if(buffer == NULL){
		return 0;
	}

	// copy buffer
	vsnprintf(buffer,len,format,args);
	buffer[len] = '\0';

	// view messagebox
	result = MessageBox(NULL,buffer,NULL,MB_OK);

	// delete buffer
	delete buffer;
	buffer = NULL;

	// exit
	va_end(args);
	return result;
}

int main()
{
	// 基本的な呼び出し方
	// 引数の順番が微妙に違うので注意が必要
	int nResult = vMessageBox(
			NULL,
			"TestTitle",
			MB_OK,
			"created by %s.",
			"kimoto"
	);

	printf("ResultValue => %d\n",nResult);
	return 0;
}
