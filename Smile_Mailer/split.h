#ifndef _SPLIT_H_INCLUDED_
#define _SPLIT_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus */

/*
 *	���[���A�h���X����h���C������؂�o��
 *	����������0��Ԃ�
 */
extern int SplitDomainName(char *address,char *dest);

/*
 *	�t���p�X���f�B���N�g���������ɂ���
 */
extern int SplitDirectoryName(char *src,char *result);

#ifdef __cplusplus
}
#endif	/*__cplusplus */

#endif	/* _SPLIT_H_INCLUDED_ */