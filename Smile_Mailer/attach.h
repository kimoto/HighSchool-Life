/*
 *	�Y�t�t�@�C���֘A�̊֐�
 *	copyright(c) 2004 kimoto
 */
#ifndef _ATTACH_H_INCLUDED_
#define ATTACH_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus */

/*�Y�t�t�@�C�������݂��邩���ׂ�*/
extern BOOL CheckFileExist(char *filepath);
/*�Y�t�t�@�C����ǂݍ���*/
extern BOOL ReadAttachFile(char *filepath,MAILDATA *mdat);


#ifdef __cplusplus
}
#endif	/* __cplusplus	*/

#endif	/* _ATTACH_H_INCLUDED_ */