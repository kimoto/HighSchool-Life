/*
 *	MX���R�[�h���擾����֐�
 *	copyright(c) 2004 kimoto
 */
#ifndef _MX_H_INCLUDED_
#define _MX_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus */

typedef struct{
	char domain[256];	/*�h���C����*/
	int priority;		/*�D��x*/
	BOOL flag;			/*���̍\���̂Ƀf�[�^�������Ă��邩*/
}MXRECORD,*LPMXRECORD;

/*
 *	MX���R�[�h���擾����֐�
 *	����������0��Ԃ�
 */
extern int GetMXRecord(char *domain,char *dns_server_addr,MXRECORD *);
#define GETMX_ERROR_SUCCESS	 0
#define GETMX_ERROR_SOCKET	-1
#define GETMX_ERROR_SENDTO	-2
#define GETMX_ERROR_RECV	-3

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/*_MX_H_INCLUDED_*/