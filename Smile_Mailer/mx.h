/*
 *	MXレコードを取得する関数
 *	copyright(c) 2004 kimoto
 */
#ifndef _MX_H_INCLUDED_
#define _MX_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus */

typedef struct{
	char domain[256];	/*ドメイン名*/
	int priority;		/*優先度*/
	BOOL flag;			/*この構造体にデータが入っているか*/
}MXRECORD,*LPMXRECORD;

/*
 *	MXレコードを取得する関数
 *	成功したら0を返す
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