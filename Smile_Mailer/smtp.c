/*
 *	���[�����M�̂��߂̊֐��Q
 *	smtp.c
 *	2004/07/17
 *	2004/07/16 : program by kimoto
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <winsock.h>
#pragma comment(lib,"wsock32.lib")
#include "smtp.h"
#include "mime.h"
#include "convert.h"

#ifdef _DEBUG
#define DebugPrint(message) MessageBox(NULL,message,"DEBUG",MB_OK);
#else
#define DebugPrint(message)
#endif

#define TCP_SMTP_PORT 25

static int SendMailSender(SOCKET sock,MAILDATA *mdat);
static int SendMailBody(SOCKET sock,MAILDATA *mdat);
static int SendMailSubject(SOCKET sock,MAILDATA *mdat);
static int SendMailAttach(SOCKET sock,MAILDATA *mdat);
static int SendMailAttachContent(SOCKET sock,char *boundary,MAILDATA *mdat);

static int vsend(SOCKET sock,char *format,...);
#define VSEND_MAX_SIZE 1024

int Check_isprint(char *buffer);

static int dump_recv(SOCKET sock);
#define DUMP_RECV_MAX_SIZE 1024

static void free_ex(void *buffer);

/*
 *	�w�肳�ꂽ���[���T�[�o�[���g����
 *	���[���𑗐M����֐�
 *	����������0��Ԃ�
 *	�G���[�R�[�h�̏ڍׂ́A�w�b�_�t�@�C���Q��
 */
extern int SendMail(char *smtp_server,MAILDATA *mdat,BOOL bAttach,char *boundary)
{
	WSADATA wsadata;
	SOCKET sock;
	SOCKADDR_IN sockaddr_in;
	LPHOSTENT lphost;
	int error_value = 0;

	DebugPrint("SendMail Start");
	WSAStartup(MAKEWORD(1,1),&wsadata);
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock == INVALID_SOCKET){
		DebugPrint("socket error");
		error_value = SENDMAIL_ERROR_SOCKET;
		goto error;
	}
	lphost = gethostbyname(smtp_server);
	if(lphost == NULL){
		DebugPrint("���̂悤�ȃT�[�o�[�͌�����܂���");
		error_value = SENDMAIL_ERROR_SMTPSERVER;
		goto error;
	}
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(TCP_SMTP_PORT);
	sockaddr_in.sin_addr = *((LPIN_ADDR)*lphost->h_addr_list);
	if(connect(sock,(SOCKADDR *)&sockaddr_in,sizeof(sockaddr_in)) != 0){
		DebugPrint("connect error");
		error_value = SENDMAIL_ERROR_CONNECT;
		goto error;
	}
	/*first reply*/
	dump_recv(sock);
	/*hello*/
	vsend(sock,"HELO %s\r\n",mdat->pc_name);
	dump_recv(sock);
	/*mail from*/
	vsend(sock,"MAIL FROM: <%s>\r\n",mdat->from);
	dump_recv(sock);
	/*rcpt to*/
	vsend(sock,"RCPT TO: <%s>\r\n",mdat->to);
	dump_recv(sock);
	/*data*/
	vsend(sock,"DATA\r\n");
	dump_recv(sock);
	/*from*/
	SendMailSender(sock,mdat);
		//vsend(sock,"From: %s <%s>\r\n",mdat->sender,mdat->from);
	/*to*/
	vsend(sock,"To: <%s>\r\n",mdat->to);
	/*x-mailer*/
	vsend(sock,"X-Mailer: %s\r\n",mdat->mailer);
	/*reply-to*/
	vsend(sock,"Reply-To: <%s>\r\n",mdat->reply);
	/*subject*/
	if(mdat->subject != NULL){
		SendMailSubject(sock,mdat);
	}
	/*mime-version*/
	vsend(sock,"MIME-Version: 1.0\r\n");
	/*priority*/
	vsend(sock,"X-Priority: %s\r\n",mdat->priority);
	/*content-type*/
	if(bAttach == TRUE){
		vsend(sock,"Content-Type: %s; boundary=\"%s\"\r\n","multipart/mixed",boundary);
	}else{
		vsend(sock,"Content-Type: %s; charset=%s","text/plain","ISO-2022-JP");
	}
	/*empty-line*/
	vsend(sock,"\r\n");
	/*�Y�t�t�@�C�������鎞�ƂȂ����̏���*/
	if(bAttach == TRUE){
		vsend(sock,"\r\n");
		vsend(sock,"--%s\r\n",boundary);
		vsend(sock,"Content-Type: text/plain; charset=ISO-2022-JP\r\n");
		vsend(sock,"\r\n");
		if(mdat->body != NULL){
			SendMailBody(sock,mdat);
		}
		SendMailAttachContent(sock,boundary,mdat);
		if(mdat->attach != NULL){
			SendMailAttach(sock,mdat);
		}
		vsend(sock,"--%s--\r\n",boundary);
	}else{
		if(mdat->body != NULL){
			vsend(sock,"\r\n");
			SendMailBody(sock,mdat);
		}
	}
	/*body end*/
	vsend(sock,"\r\n.\r\n");
	dump_recv(sock);
	/*quit*/
	vsend(sock,"QUIT\r\n");
	dump_recv(sock);

	closesocket(sock);
	WSACleanup();
	return 0;

/*�G���[����*/
error:
	if(sock != INVALID_SOCKET){
		closesocket(sock);
	}
	WSACleanup();
	return error_value;
}

/*
 *	MAILDATA�\���̂̏�����
 */
extern int InitMailData(MAILDATA *mdat)
{
	/*dynamic array*/
	mdat->attach = NULL;
	mdat->body = NULL;
	mdat->subject = NULL;
	
	memset(mdat->from,'\0',sizeof(mdat->from));
	memset(mdat->from,'\0',sizeof(mdat->from));
	memset(mdat->mailer,'\0',sizeof(mdat->mailer));
	memset(mdat->reply,'\0',sizeof(mdat->reply));
	memset(mdat->sender,'\0',sizeof(mdat->sender));
	memset(mdat->to,'\0',sizeof(mdat->to));
	memset(mdat->attach_name,'\0',sizeof(mdat->attach_name));
	memset(mdat->priority,'\0',sizeof(mdat->priority));
	mdat->attach_size = 0;

	return 0;
}

/*
 *	���[���̑��M�Җ�(mdat->sender)�𑗐M����֐�
 */
static int SendMailSender(SOCKET sock,MAILDATA *mdat)
{
	char *jis_sender = NULL;
	char *mime_sender = NULL;
	int sender_len = strlen(mdat->sender);
	
	jis_sender = malloc(sender_len * 2 + 1024);
	mime_sender = malloc(sender_len * 2 + 1024);
//	memset(mime_sender,'\0',sender_len * 2 + 1024);

	/*JIS�R�[�h�ɂ��ׂ������ׂĂ���ϊ�*/
	if(Check_isprint(mdat->sender) != 0){
		/*�����R�[�h��JIS�ɂ���*/
		SJisToJis(mdat->sender,jis_sender);
		/*MIME��ϊ�*/
		encode_to_mime_base64(jis_sender,mime_sender);
		vsend(sock,"From: %s <%s>\r\n",mime_sender,mdat->from);
	}else{
		/*�ϊ����Ȃ��Ă��悩�����炻�̂܂�ܑ��M*/
		vsend(sock,"From: %s <%s>\r\n",mdat->sender,mdat->from);
	}
	
	/*������*/
	if(jis_sender != NULL){
		free_ex(jis_sender);
	}
	if(mime_sender != NULL){
		free_ex(mime_sender);
	}
	return 0;
}


/*
 *	���[���̖{��������t�����đ��M����֐�
 */
static int SendMailBody(SOCKET sock,MAILDATA *mdat)
{
	char *body = NULL;
	int body_len;

	/*���I�Ƀ������[�����蓖�Ă�*/
	body_len = strlen(mdat->body) * 2 + 1024;
	body = (char *)malloc(body_len);
	//memset(body,'\0',body_len);

	/*JIS�R�[�h�ɂ��ׂ������ׂĂ���ϊ�*/
	if(Check_isprint(mdat->body) != 0){
		/*�����R�[�h��JIS�ɂ���*/
		SJisToJis(mdat->body,body);
		/*���H�����{���𑗐M����*/
		vsend(sock,"%s\r\n",body);
	}else{
		/*���̂܂�*/
		vsend(sock,"%s\r\n",mdat->body);
	}

	/*������*/
	if(body != NULL){
		free_ex(body);
	}
	return 0;
}

/*
 *	���[���̌��������낢����H���đ��M����֐�
 */
static int SendMailSubject(SOCKET sock,MAILDATA *mdat)
{
	char *temp = NULL;
	char *subject = NULL;
	int subject_len;

	/*���I�Ƀ������[�����蓖�Ă�*/
	subject_len = strlen(mdat->subject) * 2 + 1024;
	temp = malloc(subject_len);
	//memset(temp,'\0',subject_len);

	subject = malloc(subject_len);
	//memset(subject,'\0',subject_len);

	/*MIME�̌`���ɂ��ׂ������ׂĂ���ϊ�*/
	if(Check_isprint(mdat->subject) != 0){
		/*JIS�R�[�h�ɕϊ�*/
		SJisToJis(mdat->subject,temp);
		/*MIME�̌`����*/
		encode_to_mime_base64(temp,subject);		
		/*�����𑗐M*/
		vsend(sock,"Subject: %s\r\n",subject);
	}else{
		/*���̂܂�*/
		vsend(sock,"Subject: %s\r\n",mdat->subject);
	}

	/*������*/
	if(temp != NULL){
		free_ex(temp);
	}
	if(subject != NULL){
		free_ex(subject);
	}
	return 0;
}

/*
 *	�w�肳�ꂽ�o�b�t�@���󎚉\�����ŏo���Ă��邩���ׂ�
 *	�󎚉\�Ȃ�0��Ԃ�
 */
static int Check_isprint(char *buffer)
{
	int i = strlen(buffer);
	int result = 0;

	while(i-- > 0){
		/*�����A�󎚕s�\��������*/
		if(isprint(buffer[i]) == 0){
			return -1;
		}
	}
	return 0;
}


/*
 *	�Y�t�t�@�C���p��Content-***�𑗐M����
 */
static int SendMailAttachContent(SOCKET sock,char *boundary,MAILDATA *mdat)
{
	char jiscode[256]="";
	char buffer[256]="";

	vsend(sock,"--%s\r\n",boundary);
	/*�ϊ����Ȃ���΂Ȃ�Ȃ���*/
	if(Check_isprint(mdat->attach_name) != 0){
		/*JIS�R�[�h�ɕϊ�����*/
		SJisToJis(mdat->attach_name,jiscode);
		/*MIME�`���ɕϊ�����*/
		encode_to_mime_base64(jiscode,buffer);
	}else{
		/*���̂܂�*/
		strcpy(buffer,mdat->attach_name);
	}

	vsend(sock,"Content-Type: application/octet-stream; name=\"%s\"\r\n",buffer);
	vsend(sock,"Content-Disposition: attachment; filename=\"%s\"\r\n",buffer);
	vsend(sock,"Content-Transfer-Encoding: base64\r\n");
	vsend(sock,"\r\n");
		
	return 0;
}

/*
 *	���[���̓Y�t�t�@�C�������낢��Ɖ��H���đ��M����֐�
 */
static int SendMailAttach(SOCKET sock,MAILDATA *mdat)
{
	char *attach_file;
	int attach_size;
	int base64_attach_size;

	attach_size = mdat->attach_size * 2 + 1024;
	attach_file = (char *)malloc(attach_size);
	memset(attach_file,'\0',attach_size);

	/*base64�ŃG���R�[�h����*/
	base64_attach_size = encode_to_base64(mdat->attach,attach_file,mdat->attach_size);

	/*���M���� - �o�C�i���f�[�^�𑗂�̂ŁA�T�d��*/
	send(sock,attach_file,base64_attach_size,0);
	vsend(sock,"\r\n");
	DebugPrint(attach_file);

	/*�������*/
	if(attach_file != NULL){
		free_ex(attach_file);
	}
	return 0;
}


/*
 *	�ό�����send�֐�
 *	send�֐��̖߂�l�����̂܂ܕԂ�
 */
static int vsend(SOCKET sock,char *format,...)
{
	char buffer[VSEND_MAX_SIZE]="";
	int ret;
	va_list ap;

	va_start(ap,format);

	vsprintf(buffer,format,ap);
	DebugPrint(buffer);
	ret = send(sock,buffer,strlen(buffer),0);

	va_end(ap);
	return ret;
}

/*
 *	��M�����f�[�^���̂Ă�recv�֐�
 *	recv�֐��̖߂�l�����̂܂ܕԂ�
 */
static int dump_recv(SOCKET sock)
{
	char buffer[DUMP_RECV_MAX_SIZE]="";
	int recv_val;

	recv_val = recv(sock,buffer,sizeof(buffer),0);
	DebugPrint(buffer);

	return recv_val;
}

/*
 *	free�֐��̊g����
 *	NULL�������邱�Ƃɂ���ĊԈႦ��
 *	���łɉ�������������[�ɃA�N�Z�X��������
 *	�ɃG���[���o��悤�ɂ���
 */
static void free_ex(void *buffer)
{
	free(buffer);
	buffer = NULL;
}

