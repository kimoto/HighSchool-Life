/*
 *	���[�����M�̊֐��Q
 *	BASE64�ւ̃G���R�[�h�����s���B�������A�֐��̎��͕̂ʂ̃t�@�C���B
 *	copyright(c) 2004 kimoto
 */
#ifndef _SMTP_H_INCLUDED_
#define _SMTP_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus */

#define MAILBUF_SIZE 256

typedef struct{
	char *body;					/*���[���̖{�� ���I�z��*/
	char *subject;				/*���[���̌��� ���I�z��*/
	char *attach;				/*�Y�t�t�@�C�� ���I�z��*/
	char attach_name[MAILBUF_SIZE];	/*�Y�t�t�@�C���̖��O*/
	size_t attach_size;			/*�Y�t�t�@�C���̃T�C�Y*/
	char mailer[MAILBUF_SIZE];	/*���[���[�̖��O*/
	char sender[MAILBUF_SIZE];	/*���M�Җ�*/
	char from[MAILBUF_SIZE];	/*���M���A�h���X*/
	char to[MAILBUF_SIZE];		/*���M��A�h���X*/
	char reply[MAILBUF_SIZE];	/*�ԐM��A�h���X*/
	char priority[MAILBUF_SIZE];/*�D��x*/
	char pc_name[MAILBUF_SIZE];/*���̃\�t�g���g�p����R���s���[�^�̖��O*/
}MAILDATA,*LPMAILDATA;

/*���[���𑗐M����֐�*/
extern int SendMail(char *smtp_server,MAILDATA *mdat,BOOL bAttach,char *boundary);
#define SENDMAIL_ERROR_SUCCESS 0		/*����������*/
#define SENDMAIL_ERROR_SOCKET -1		/*�\�P�b�g�쐬���s*/
#define SENDMAIL_ERROR_SMTPSERVER -2	/*SMTP�T�[�o�[��������Ȃ�*/
#define SENDMAIL_ERROR_CONNECT -3		/*SMTP�T�[�o�[�Ƃ̐ڑ����s*/

/*MAILDATA�\���̂�����������֐�*/
extern int InitMailData(MAILDATA *mdat);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif _SMTP_H_INCLUDED_	/* _SMTP_H_INCLUDED_ */