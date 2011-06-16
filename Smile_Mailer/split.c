/*
 *	�����񑀍�ŁA���o������ϊ�������Ƃ������֐�
 */
#include <string.h>
#include "split.h"

/*
 *	���[���A�h���X����h���C������؂�o��
 *	����������0��Ԃ�
 */
extern int SplitDomainName(char *address,char *dest)
{
	if((address == NULL) || (dest == NULL)){
		return -1;
	}
	
	while(*address != '\0'){
		if(*address == '@'){
			address++;
			strcpy(dest,address);
			break;
		}
		address++;
	}
	return 0;
}


/*
 *	�t���p�X�����璼�O�̃f�B���N�g���������ɂ���
 */
extern int SplitDirectoryName(char *filepath,char *result)
{
	int len = strlen(filepath);

	while(len-- > 0){
		if(filepath[len] == '\\'){
			//filepath[len + 1] = '\0';
			strncpy(result,filepath,len);
			return 0;
		}
	}
	return -1;
}
