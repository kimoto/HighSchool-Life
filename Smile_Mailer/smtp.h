/*
 *	メール送信の関数群
 *	BASE64へのエンコード等も行う。ただし、関数の実体は別のファイル。
 *	copyright(c) 2004 kimoto
 */
#ifndef _SMTP_H_INCLUDED_
#define _SMTP_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus */

#define MAILBUF_SIZE 256

typedef struct{
	char *body;					/*メールの本文 動的配列*/
	char *subject;				/*メールの件名 動的配列*/
	char *attach;				/*添付ファイル 動的配列*/
	char attach_name[MAILBUF_SIZE];	/*添付ファイルの名前*/
	size_t attach_size;			/*添付ファイルのサイズ*/
	char mailer[MAILBUF_SIZE];	/*メーラーの名前*/
	char sender[MAILBUF_SIZE];	/*発信者名*/
	char from[MAILBUF_SIZE];	/*送信元アドレス*/
	char to[MAILBUF_SIZE];		/*送信先アドレス*/
	char reply[MAILBUF_SIZE];	/*返信先アドレス*/
	char priority[MAILBUF_SIZE];/*優先度*/
	char pc_name[MAILBUF_SIZE];/*このソフトを使用するコンピュータの名前*/
}MAILDATA,*LPMAILDATA;

/*メールを送信する関数*/
extern int SendMail(char *smtp_server,MAILDATA *mdat,BOOL bAttach,char *boundary);
#define SENDMAIL_ERROR_SUCCESS 0		/*成功した時*/
#define SENDMAIL_ERROR_SOCKET -1		/*ソケット作成失敗*/
#define SENDMAIL_ERROR_SMTPSERVER -2	/*SMTPサーバーが見つからない*/
#define SENDMAIL_ERROR_CONNECT -3		/*SMTPサーバーとの接続失敗*/

/*MAILDATA構造体を初期化する関数*/
extern int InitMailData(MAILDATA *mdat);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif _SMTP_H_INCLUDED_	/* _SMTP_H_INCLUDED_ */