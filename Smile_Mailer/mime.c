/*
 *	MIME�`���ɃG���R�[�h���s���֐��Q
 *	�f�R�[�h�̓T�|�[�g���Ă��Ȃ�
 *	mime.c
 *	2004/07/14 : program by kimoto
 */
#include <stdio.h>
#include <string.h>
#include "mime.h"

typedef unsigned long u_long;
typedef unsigned char u_char;

static char convert_to_base(const char c);

/*
 *	MIME�`����base64�ɕϊ�����
 *	�����������������̃o�b�t�@��傫�����Ȃ���΂Ȃ�Ȃ�
 *	base64�ϊ��ɂ���ĕ�����̃T�C�Y�������邽��
 */
extern size_t encode_to_mime_base64(const char *src,char *dest)
{
	size_t total_size = 0;

	/*�w�b�_�݂����Ȃ̂�ǉ�*/
	strcpy(dest,"=?ISO-2022-JP?B?");
	total_size += strlen(dest);
	dest += total_size;

	total_size += encode_to_base64(src,dest,strlen(src));

	/*�I���t���O�݂����̂�ǉ�*/
	strcat(dest,"?=");
	total_size += 2;

	return total_size;
}

/*
 *	base64�ɕϊ�����
 *	�߂�l�́A�ϊ���̃T�C�Y
 *	������NULL��������
 */
extern size_t encode_to_base64(const u_char *src,u_char *dest,int size)
{
	u_long bb = 0;
	int cnt = 0;
	int i = 0;
	size_t total_size = 0;

	for(i=0;i<size;i++){
		cnt++;
		bb <<= 8;
		bb |= *src;

		/*3���*/
		if(cnt == 3){
			*dest++ = convert_to_base((char)((bb&(0x3F<<18))>>18));
			*dest++ = convert_to_base((char)((bb&(0x3F<<12))>>12));
			*dest++ = convert_to_base((char)((bb&(0x3F<< 6))>> 6));
			*dest++ = convert_to_base((char)((bb&(0x3F<< 0))>> 0));
			total_size += 4;
			bb = 0;
			cnt = 0;
		}
		src++;
	}
	/*2���*/
	if(cnt == 2){
		bb <<= 8;
		*dest++ = convert_to_base((char)((bb&(0x3F<<18))>>18));
		*dest++ = convert_to_base((char)((bb&(0x3F<<12))>>12));
		*dest++ = convert_to_base((char)((bb&(0x3F<< 6))>> 6));
		*dest++ = '=';
		total_size += 4;
	}
	/*1���*/
	if(cnt == 1){
		bb <<= 16;
		*dest++ = convert_to_base((char)((bb&(0x3F<<18))>>18));
		*dest++ = convert_to_base((char)((bb&(0x3F<<12))>>12));
		*dest++ = '=';
		*dest++ = '=';
		total_size += 4;
	}
	/*������NULL������ǉ�*/
	*dest = '\0';

	return total_size;
}


/*
 *	�x�[�X�ƂȂ�A���t�@�x�b�g���ɕϊ�����
 */
static char convert_to_base(const char c)
{
    if (c <= 0x19)
        return c + 'A';
    if (c >= 0x1a && c <= 0x33)
        return c - 0x1a + 'a';
    if (c >= 0x34 && c <= 0x3d)
        return c - 0x34 + '0';
    if (c == 0x3e)
        return '+';
    if (c == 0x3f)
        return '/';
    return '=';
}