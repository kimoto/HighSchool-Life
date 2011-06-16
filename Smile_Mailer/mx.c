/*
*	DNS�T�[�o�[����MX���R�[�h���擾���邽�߂̊֐��Q
*	mx.c
*	2004/07/21
*	2004/07/14
*	2004/07/11
*	2004/07/10 program by kimoto
*/
#include <stdio.h>
#include <winsock.h>
#pragma comment(lib,"wsock32.lib")
#include "mx.h"

//#define DEBUG_MODE

#ifdef DEBUG_MODE
#define DebugPrint(message) fprintf(stderr,"%s\n",message);
#else
#define DebugPrint(message)
#endif

/*	�Ɖ�^�C�v	*/
#define DNS_TYPE_A		 1
#define DNS_TYPE_NS		 2
#define DNS_TYPE_CNAME	 5
#define DNS_TYPE_PTR	12
#define DNS_TYPE_MX		15
#define DNS_TYPE_AAAA	28
#define DNS_TYPE_ANY	255

/*	�Ɖ�N���X	*/
#define DNS_CLASS_IN	 1

#define DNS_RD_RECURSION 0x100

#define UDP_DNS_PORT 53

#define CHECK_RCODE_ERROR (1 | 2 | 3 | 4 | 5)

/*MakeDNSPacket relation*/
static size_t MakeDNSPacket(u_short dns_id,const char *domain,char *send_buf,u_short dns_flag,
							u_short dns_type,u_short dns_class);
static size_t MakeDNSQuery(char *packet,const char *domain,u_short dns_type,u_short dns_class);
static size_t MakeDNSQueryLabel(const char *src,char *dest);

/*ExpandDNSPacket relation*/
static int ExpandDNSPacket(char *packet,int dns_id,MXRECORD *mx);
#define ERROR_ID_NUMBER		-1	/*���ʔԍ����Ⴄ*/

/*	FLAG ERROR */
#define RCODE_ERROR_NONE	0	/*�G���[�Ȃ�*/
#define RCODE_ERROR_FORMAT	1	/*�\���G���[*/
#define RCODE_ERROR_SERVER	2	/*�T�[�o�[�̃G���[*/
#define RCODE_ERROR_NAME	3	/*�l�[���G���[*/
#define RCODE_ERROR_XXXX	4	/*������*/
#define RCODE_ERROR_REFUSAL	5	/*���ۂ��ꂽ*/
#define RCODE_ERROR			6	/*���m�̃G���[*/

/* other */
static int SkipQuery(char **src,int n);
#define ERROR_NULL_PTR	-1
static int GetMXFromAnswer(char *packet,char *pch,int answer_num,MXRECORD *mx);
#define ERROR_NOT_MX	-1
static char *ExpandDNSLabel(char *top,char *source,char *result);
static int CheckDNSFlag(int flag);

/*
*	MX���R�[�h���擾����֐�
*	�O���[�o���֐��́A���ꂾ��
*/
extern int GetMXRecord(char *domain,
					   char *dns_server_addr,
					   MXRECORD *mx)
{
	WSADATA wsadata;
	SOCKET sock;
	char send_buf[1024]="";
	char recv_buf[1024]="";
	int packet_size;
	int fromlen;
	SOCKADDR_IN sockaddr_in;
	WSAStartup(MAKEWORD(1,1),&wsadata);
	
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == INVALID_SOCKET){
		return GETMX_ERROR_SOCKET;
	}
	packet_size = MakeDNSPacket(111,domain,send_buf,
		DNS_RD_RECURSION,DNS_TYPE_MX,DNS_CLASS_IN);
	/*DNS�T�[�o�[�ւ̐ڑ��̏���*/
	sockaddr_in.sin_addr.S_un.S_addr = inet_addr(dns_server_addr);
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(UDP_DNS_PORT);
	
	/*DNS�p�P�b�g���T�[�o�[�ɑ��M����*/
	if(SOCKET_ERROR == sendto(sock,send_buf,packet_size,0,
		(SOCKADDR *)&sockaddr_in,sizeof(sockaddr_in))){
		return GETMX_ERROR_SENDTO;
	}
	fromlen = sizeof(sockaddr_in);
	/*�T�[�o�[����̉�������M����*/
	if(SOCKET_ERROR == recvfrom(sock,recv_buf,sizeof(recv_buf),0,
		(SOCKADDR *)&sockaddr_in,&fromlen)){
		return GETMX_ERROR_RECV;
	}
	/*��M�����f�[�^����́A�擾����*/
	ExpandDNSPacket(recv_buf,111,mx);
	
	closesocket(sock);
	WSACleanup();
	return GETMX_ERROR_SUCCESS;
}


/*-------------------------------------------------------
* �֐��� : MakeDNSPacket
*-------------------------------------------------------
* ����
*	DNS�p�P�b�g���쐬����֐�
*	�قƂ�ǂ��ׂĂ̌`���ɑΉ�����
*
* ����
*	dns_id		:	���ʔԍ�(�p�P�b�gID�ԍ�)
*	domain		:	�Ɖ�����h���C�������w�肷��
*	send_buf	:	���M�o�b�t�@���w��
*	dns_flag	:	�����E�����R�[�h�̃t���O
*	dns_type	:	DNS�ɗv�����郊�\�[�X���R�[�h�̃^�C�v(MX,A,AAAA,NS��)
*	dns_class	:	DNS�̃N���X(inter net��)
* �߂�l
*	�쐬����DNS�p�P�b�g�̑��T�C�Y
*-------------------------------------------------------
* ����
*	2004/07/06 : program by kimoto
*-------------------------------------------------------
*/
static size_t MakeDNSPacket(u_short dns_id,
							const char *domain,
							char *send_buf,
							u_short	dns_flag,
							u_short	dns_type,
							u_short	dns_class)
{
	size_t total_size;
	/*16bit(2byte)�P�ʂŃA�N�Z�X���邽�߂̃|�C���^*/
	u_short *us_ptr = (u_short*)send_buf;
	
	us_ptr[0] = htons(dns_id);		/*���ʔԍ�*/
	us_ptr[1] = htons(dns_flag);	/*�����E�����R�[�h(�ċA�I�₢���킹)*/
	us_ptr[2] = htons(1);			/*���╔�̗v�f��*/
	us_ptr[3] = htons(0);			/*�񓚕��̗v�f��*/
	us_ptr[4] = htons(0);			/*�I�[�\���e�B���̗v�f��*/
	us_ptr[5] = htons(0);			/*�ǉ���񕔂̗v�f��*/
	total_size = 12;
	
	/*���╔���쐬*/
	total_size += MakeDNSQuery((char *)&us_ptr[6],domain,dns_type,dns_class);
	
	return total_size;
}

/*-------------------------------------------------------
* �֐��� : ExpandDNSPacket
*-------------------------------------------------------
* ����
* �w�肳�ꂽDNS�p�P�b�g����MX���R�[�h�̏���
* ���o����֐�
*
* ����
*	packet : DNS�p�P�b�g
*	dns_id : DNS�p�P�b�g���M���ɕt�������j�[�N�Ȕԍ�
*	mx     : ���o�����f�[�^���i�[����\����
* �߂�l
*	ERROR_SUCCESS		 0 : �֐�������������
*	ERROR_ID_NUMBER		-1 : ���ʔԍ��Ɉُ킪������
*	error_code	:	�t���O�̃G���[�����̂܂ܕԂ�
*-------------------------------------------------------
* ����
* 2004/07/05 : program by kimoto
*-------------------------------------------------------
*/
static int ExpandDNSPacket(char *packet,
						   int	dns_id,
						   MXRECORD *mx)
{
	/*2byte,1byte�P�ʂŃA�N�Z�X���邽�߂̃|�C���^*/
	u_short *us_ptr;
	char *pch;
	
	int questions_num;			/*���⃌�R�[�h��*/
	int answer_num;				/*�񓚃��R�[�h��*/
	int authority_num;			/*���Ѓ��R�[�h��*/
	int additional_num;			/*�ǉ����R�[�h��*/
	
	int error_code;				/*�t���O�̃G���[�R�[�h*/
	
	us_ptr = (u_short *)packet;
	/*���ʔԍ��̃`�F�b�N*/
	if(ntohs(us_ptr[0]) != dns_id){
		DebugPrint("Packet�̎��ʔԍ����A���M���̂��̂ƈႢ�܂��B");
		return ERROR_ID_NUMBER;
	}
	/*�t���O�̃`�F�b�N*/
	error_code = CheckDNSFlag(ntohs(us_ptr[1]));
	if(error_code != RCODE_ERROR_NONE){
		return error_code;
	}
	DebugPrint("����ȃp�P�b�g�ł��B");
	
	/*���␔�A�񓚐��A���А��A�ǉ�RR���̎擾*/
	questions_num = ntohs(us_ptr[2]);
	answer_num = ntohs(us_ptr[3]);
	authority_num = ntohs(us_ptr[4]);
	additional_num = ntohs(us_ptr[5]);
	
	/*���╔���R�[�h�̊J�n�_*/
	pch = (char *)&us_ptr[6];
	
	/*���╔���R�[�h�̓ǂݔ�΂�*/
	SkipQuery(&pch,questions_num);
	
	/*�񓚕����R�[�h�̎擾*/
	GetMXFromAnswer(packet,pch,answer_num,mx);
	
	//���Е����R�[�h�A�ǉ���񕔃��R�[�h�́A�g��Ȃ��B
	
	return ERROR_SUCCESS;
}


/*-------------------------------------------------------
* �֐��� : CheckDNSFlag
*-------------------------------------------------------
* ����
*	DNS�p�P�b�g�̃t���O�𒲂ׂ�֐�
*
* ����
*	flag	:	�T�[�o�[����͂����f�[�^
* �߂�l
*	error_code	:	�擾�����G���[�R�[�h��Ԃ�
*-------------------------------------------------------
* ����
*	2004/07/05 : program by kimoto
*-------------------------------------------------------
*/
static int CheckDNSFlag(int flag)
{
	return (flag & CHECK_RCODE_ERROR);
}

/*-------------------------------------------------------
* �֐��� : SkipQuery
*-------------------------------------------------------
* ����
*	�w�肳�ꂽ�ʒu����n�܂鎿�╔���R�[�h���X�L�b�v����
*
* ����
*	src	:	���╔���R�[�h�̐擪�ʒu���w��
*	n	:	���╔���R�[�h�̌����w��
* �߂�l
*	ERROR_SUCCESS	 0 : �֐�������������
*	ERROR_NULL		-1 : ����src��NULL���w���Ă����Ƃ�
*-------------------------------------------------------
* ����
*	2004/07/05 : program by kimoto
*-------------------------------------------------------
*/
static int SkipQuery(char **src,int	n)
{
	int len;
	
	if(*src == NULL){
		return ERROR_NULL_PTR;
	}
	
	/*���╔���R�[�h�̌������[�v*/
	while(n-- > 0){
		while((len=**src) != 0){
			*src = *src + 1;
			*src = *src + len;	
		}
		/*������0�̕��i�߂�*/
		*src = *src + 1;
		/*QTYPE + QCLASS = 4byte*/
		*src = *src + 4;
	}
	return ERROR_SUCCESS;
}


/*-------------------------------------------------------
* �֐��� : GetMXFromAnswer
*-------------------------------------------------------
* ����
*	�񓚕����R�[�h����AMX���\�[�X���R�[�h��
*	�h���C����(domain)�A�D��x(priority)���擾����֐�
*
*	us_ptr[0]�́A�^�C�v
*	us_ptr[1]�́A�N���X
*	us_ptr[2],[3]�́A��������(TTL)
*	us_ptr[4]�́A���\�[�X�f�[�^�̒���
*	us_ptr[5]�́AMX���R�[�h�̏ꍇ�A�D��x
*	us_ptr[6]�́AMX���R�[�h�̏ꍇ�A�h���C����
*
* ����
*	packet	:	��M����DNS�p�P�b�g�̐擪�ʒu
*	pch		:	�񓚕����R�[�h�̐擪�ʒu
*	answer	:	�񓚕����R�[�h�̌�
*	mx		:	�擾�����f�[�^���i�[���邽�߂̍\����
*
* �߂�l
*	ERROR_SUCCESS		 0 : �֐�������������
*	ERROR_NOT_MX		-1 : MX���R�[�h�ł͂Ȃ�������
*-------------------------------------------------------
* ����
*	2004/07/05 : program by kimoto
*-------------------------------------------------------
*/
static int GetMXFromAnswer(char		*packet,
						   char		*pch,
						   int		answer_num,
						   MXRECORD	*mx)
{
	char buffer[256]="";
	u_short *us_ptr;
	
	while(answer_num-- > 0){
		/*�Ɖ�x���̎擾*/
		pch = ExpandDNSLabel(packet,pch,buffer);
		DebugPrint(buffer);
		
		/*���̃��R�[�h�̃^�C�v���AMX���R�[�h�����ׂ�*/
		us_ptr = (u_short *)pch;
		if(ntohs(us_ptr[0]) != DNS_TYPE_MX){
			return ERROR_NOT_MX;
		}
		
		/*�D��x���擾*/
		mx->priority = ntohs(us_ptr[5]);
		
		/*�h���C�������擾*/
		pch = ExpandDNSLabel(packet,(char *)&us_ptr[6],mx->domain);
		
		/*�f�[�^����ꂽ�̂ŁA�t���O�𗧂Ă�*/
		mx->flag = TRUE;
		mx++;
	}
	
	/*�f�[�^������\�����߁AFALSE���i�[*/
	mx->flag = FALSE;
	
	return ERROR_SUCCESS;
}


/*-------------------------------------------------------
* �֐��� : ExpandDNSLabel
*-------------------------------------------------------
* ����
*	���k���ꂽ���x����W�J����֐�
*
* ����
*	packet	:	��M����DNSPacket�̐擪�ʒu
*	label	:	�W�J���������x���̐擪�ʒu
*	result	:	���ʂ��o�͂���o�b�t�@
* �߂�l
*	�W�J�������i�߂�DNSPacket�ւ̃|�C���^
*-------------------------------------------------------
* ����
*	2004/07/05 : program by kimoto
*-------------------------------------------------------
*/
static char *ExpandDNSLabel(char *packet,
							char *label,
							char *result)
{
	int len,i;
	char *temp;
	DebugPrint(">>ExpandDNSLabel");
	
	while((len = *label++) != 0){
		/*���k����Ă�����*/
		if((len & 0xC0) == 0xC0){
			temp = packet + *label;
			ExpandDNSLabel(packet,temp,result);
			return ++label;
		}
		for(i=0;i<len;i++){
			*result++ = *label++;
		}
		*result++ = '.';
	}
	result--;
	*result = '\0';
	return label;
}


/*-------------------------------------------------------
* �֐��� : MakeDNSQuery
*-------------------------------------------------------
* ����
*	DNS�p�P�b�g�̎��╔���R�[�h���쐬����֐�
*
* ����
*	packet		:	DNS�p�P�b�g�̐擪�ʒu���w�肷��
*	domain		:	�Ɖ�����h���C�������w�肷��
*	dns_type	:	�Ɖ�����^�C�v���w�肷��(MX,A,AAAA,NS��)
*	dns_class	:	�Ɖ�����N���X���w�肷��(internet��)
* �߂�l
*	���╔���R�[�h�̑��T�C�Y��Ԃ�
*-------------------------------------------------------
* ����
*	2004/07/06 : program by kimoto
*-------------------------------------------------------
*/
static size_t MakeDNSQuery(char *packet,
						   const char *domain,
						   u_short dns_type,
						   u_short dns_class)
{
	int total_size = 0;
	u_short *us_ptr;
	
	/*�Ɖ�x�����쐬*/
	total_size = MakeDNSQueryLabel(domain,packet);
	packet += total_size;
	
	/*�Ɖ�^�C�v�A�N���X���쐬*/
	us_ptr = (u_short *)packet;
	us_ptr[0] = htons(dns_type);
	us_ptr[1] = htons(dns_class);
	
	/*QTYPE = 2byte,QCLASS = 2byte*/
	return total_size + 2 + 2;
}


/*-------------------------------------------------------
* �֐��� : MakeDNSQueryLabel
*-------------------------------------------------------
* ����
*	�w�肳�ꂽ�h���C��������Ɖ�p�̃��x���̍쐬
*
* ����
*	src	:	�h���C�������w�肷��
*	result	:	���ʂ��o�͂���o�b�t�@���w�肷��
*
* �߂�l
*	�쐬�����Ɖ�x���̃T�C�Y��Ԃ�
*-------------------------------------------------------
* ����
*	2004/07/06 : program by kimoto
*-------------------------------------------------------
*/
static size_t MakeDNSQueryLabel(const char *src,char *dest)
{
	int cnt = 0;
	int i = 0;
	const char *strptr = src;
	char *dest_top = dest;
	
	while(*strptr != '\0'){
		/*.������������*/
		if(*strptr == '.'){
			*dest++ = cnt;
			for(i=0;i<cnt;i++){
				*dest++ = *src++;
			}
			cnt = 0;
			strptr++;
			src++;
			continue;
		}
		cnt++;
		strptr++;
	}
	*dest++ = cnt;
	for(i=0;i<cnt;i++){
		*dest++ = *src++;
	}
	/*������0�ƁANULL����*/
	*dest++ = 0;
	*dest = '\0';
	
	/*strlen�̌��ʂɁA0�̕���t������*/
	return strlen(dest_top) + 1;
}
