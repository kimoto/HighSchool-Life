/*
 *	�����R�[�h�ϊ��Ɋ֘A����֐�
 *	Shift-Jis����Jis�R�[�h�ւ̕ϊ��ɂ����Ή����Ă��Ȃ��i���[�����M�ł͏\���j
 *
 *	2004/07/21 : by kimoto
 */
#include <windows.h>
#include <mbstring.h>
#include "convert.h"

static int convert_SJisToJis(const char *src,char *dest);

/*
 *	SJis����JIS�ɕϊ�����A�߂�l�́A�ϊ���̃T�C�Y
 */
extern size_t SJisToJis(const char *src,char *dest)
{
	if(convert_SJisToJis(src,dest) != 0){
		return -1;
	}
	return lstrlen(dest);
}

/*
 *	Shift-JIS����JIS�ɕϊ�����
 *	����������߂�l��0
 *	������'\0'��������
 */
static int convert_SJisToJis(const char *src,char *dest)
{
	int shift_flag = FALSE;

	/*�܂��A���ʂ̃A���t�@�x�b�g�̎��́A���ʂɏo��*/
	while(*src != '\0'){
		/*���ʂ�ASCII�̎��͕��ʂɏo��*/
		if(_ismbblead(*src) == 0){
			*dest++ = *src++;
			continue;
		}
		/*SJIS����������J�n�R�[�h���o��*/
		if(_ismbblead(*src) != 0){
			/*�P�O�̕������ASJIS����Ȃ�������*/
			if(shift_flag == FALSE){
				*dest++ = 0x1b;
				*dest++ = 0x24;
				*dest++ = 0x42;
			}
			*dest++ = HIBYTE(_mbcjmstojis(MAKEWORD(*(src+1),*src)));
			*dest++ = LOBYTE(_mbcjmstojis(MAKEWORD(*(src+1),*src)));
			/*���̕������܂��ASJIS��������܂��I���R�[�h�����Ȃ��B*/
			/*�t�Ɍ����΁A���̕�����JIS�ł͂Ȃ�������I���R�[�h������*/
			src+=2;	/*���̕�����*/
			shift_flag = TRUE;
			if(!(_ismbblead(*src) != 0)){
				*dest++ = 0x1b;
				*dest++ = 0x28;
				*dest++ = 0x42;
				shift_flag = FALSE;
				continue;
			}
		}
	}
	/*������NULL������t����*/
	*dest = '\0';
	return 0;
}