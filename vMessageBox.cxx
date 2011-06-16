#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// win�Ķ�����ʤ����Ŭ����
typedef int* HWND;
typedef char* LPSTR;
typedef unsigned int UINT;
#define MB_OK		0
#define MB_CANCEL	-1

#define MessageBox(a,b,c,d) puts(b);

// ���Ѹİ�����å������ܥå���
// ����ͤϡ���å������ܥå����Τ򤽤Τޤ��֤�
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
	// ����Ū�ʸƤӽФ���
	// �����ν��֤���̯�˰㤦�Τ���դ�ɬ��
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
