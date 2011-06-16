/*
 *	DNS�N���C�A���g�v���O����
 *	2004/07/04
 *	program by kimoto
 *
 */
#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <stdio.h>
#include <winsock.h>
#pragma comment(lib,"wsock32.lib")

/*DEBUG*/
#ifdef _DEBUG
#define Debug(x) x
#define DebugPrint(message) printf("%s\n",message)
#define ErrorMessage(message) \
	fprintf(stderr,"Error - %s!!\n>>Exit Program!!\n",message);return -1;
#else
#define Debug(x)
#define DebugPrint(message)
#define ErrorMessage(message)
#endif

#define UDP_DNS_PORT 53

#define MAX_BUFFER_SIZE 2048
#define MAX_PACKET_SIZE 1024

#define DNS_RD_RECURSION 0x100

#define DNS_CLASS_IN 1

#define DNS_TYPE_A		1
#define DNS_TYPE_NS		2
#define DNS_TYPE_CNAME	5
#define DNS_TYPE_PTR	12
#define DNS_TYPE_MX		15
#define DNS_TYPE_AAAA	28
#define DNS_TYPE_ANY	255

/*DNS RESOURCE RECORD*/
typedef struct{
	u_short	dns_type;		/*�^�C�v*/
	u_short	dns_class;		/*�N���X*/
	int		priority;		/*�D��x*/
	int		ip_addr;		/*IP�A�h���X*/
	char	domain[256];	/*�h���C����*/
	char	query[256];		/*�Ɖ�x��*/
	BOOL	flag;			/*�f�[�^�������Ă��邩*/
}RES_RECORD;

/*MAKE DNS PACKET*/
size_t	MakeDNSPacket(u_short dns_id,char *domain,
					  char *send_buf,u_short,u_short,u_short);
size_t	MakeDNSQuery(char *packet,char *domain,
					 u_short dns_type,u_short dns_class);
size_t	MakeDNSQueryLabel(char *src,char *rst);

/*EXPAND DNS PACKET*/
int		ExpandDNSPacket(char *recv_buf,int dns_id,RES_RECORD *res);
int		ExpandDNSRecords(char *packet,char *pch,RES_RECORD *res,int query,
					 int answer,int authority,int add);
char	*ExpandDNSResData(char *packet,char *strptr,RES_RECORD *res);
char	*ExpandDNSResRecords(char *packet,char *top_ptr,RES_RECORD *res);
char	*SkipQuestionsRecord(char *packet,char *top_ptr);
char	*ExpandDNSLabel(char *packet,char *name_ptr,char *result);

/*GET DNS Data*/
int	GetDNSData(char *dns_server,char *domain,int dns_id,int dns_type,char *recv_buf,int recv_buf_len);

/*
 *	���C���֐�
 */
int main(int argc,char *argv[])
{
	char packet[MAX_BUFFER_SIZE]="";
	RES_RECORD res[20];

	/*DNS�f�[�^�̎�M*/
	GetDNSData("192.168.0.1","yahoo.co.jp",111,DNS_TYPE_MX,packet,sizeof(packet));
	/*������*/
	ZeroMemory(&res,sizeof(res));
	/*��M�����f�[�^�̉��*/
	ExpandDNSPacket(packet,111,res);

	int i = 0;
	printf("***********************************\n");
	printf("SERVER \n");
	//MX���R�[�h�݂̂����ʂƂ��ĕ\������
	while(res[i].flag == TRUE){
		if(res[i].dns_type == DNS_TYPE_MX){
			printf("%s - %d\n",res[i].domain,res[i].priority);
		}
		i++;
	}
	printf("************************************\n");

	return 0;
}

/*
 *	DNS�f�[�^����M����v���O����
 */
int GetDNSData(char *dns_server,
			   char *domain,
			   int	dns_id,
			   int dns_type,
			   char *recv_buf,
			   int recv_buf_size)
{
	WSADATA wsadata;
	SOCKET sock;
	SOCKADDR_IN sockaddr_in;
	char send_buf[256]="";
	int send_size = 0;
	int return_val = 0;
	int from_len = 0;

	WSAStartup(MAKEWORD(1,1),&wsadata);

	/*DNS�p�\�P�b�g�̍쐬(UDP)*/
	sock = socket(PF_INET,SOCK_DGRAM,0);
	if(sock == INVALID_SOCKET){
		ErrorMessage("socket error");
	}
	/*DNS�p�P�b�g�̍쐬*/
	send_size = MakeDNSPacket(dns_id,domain,
		send_buf,DNS_RD_RECURSION,dns_type,DNS_CLASS_IN);
	/*DNS�p�P�b�g�̑��M�̂��߂̐ݒ�*/
	sockaddr_in.sin_addr.S_un.S_addr = inet_addr(dns_server);//DNSServerAddress
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(UDP_DNS_PORT);
	/*DNS�p�P�b�g�̑��M*/
	return_val = sendto(sock,send_buf,send_size,0,
		(SOCKADDR *)&sockaddr_in,sizeof(sockaddr_in));
	if(return_val == SOCKET_ERROR){
		ErrorMessage("sendto error");
	}
	/*DNS�p�P�b�g�̎�M*/
	from_len = sizeof(sockaddr_in);
	return_val = recvfrom(sock,recv_buf,recv_buf_size,0,
		(SOCKADDR *)&sockaddr_in,&from_len);
	if(return_val == SOCKET_ERROR){
		ErrorMessage("recvfrom error");
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}

/*
 *	DNS�p�P�b�g���쐬����֐�
 *	�قƂ�ǂ��ׂĂ̌`���ɑΉ�����B
 *
 *	2004/06/27 program by kimoto
 */
size_t MakeDNSPacket(u_short	dns_id,		/*�p�P�b�gID�ԍ�*/
					 char		*domain, 	/*�h���C����*/
					 char		*send_buf,	/*���M�o�b�t�@*/
					 u_short	dns_flag,	/*�����E�����R�[�h�̃t���O*/
					 u_short	dns_type,	/*DNS�ɗv�����郌�R�[�h�̃^�C�v(mx,a��)*/
					 u_short	dns_class)	/*DNS�̃N���X*/
{
	u_short *us_ptr;
	size_t total_size;
	
	/*16bit(2byte)�ŃA�N�Z�X���邽�߂̃|�C���^*/
	us_ptr = (u_short *)send_buf;

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


/*
 *	DNS�p�P�b�g�̎��⃌�R�[�h���쐬����֐�
 */
size_t MakeDNSQuery(char *packet,
					char *domain,
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
	
	/*�Ɖ�x����2byte,�Ɖ�N���X��2byte*/
	return total_size + 2 + 2;
}


/*
 *	�w�肳�ꂽ�h���C��������Ɖ�p���x�����쐬����֐�
 */
size_t MakeDNSQueryLabel(char *src,
						 char *result)
{
	char *token;
	char *result_ptr;
	int token_len;
	int total_size = 0;
	int i;
	char buffer[256]="";
	strcpy(buffer,src);
	
	result_ptr = result;
	token = strtok(buffer,".");
	while(token != NULL){
		token_len = strlen(token);
		total_size += token_len;
		/*token_len���o�͂���*/
		*result_ptr++ = token_len;
		total_size++;
		/*token���o�͂���*/
		for(i=0;i<token_len;i++){
			*result_ptr++ = *token++;
		}
		token = strtok(NULL,".");
	}
	/*�����́A0*/
	*result_ptr++ = 0;
	total_size++;

	*result_ptr++ = '\0';
	
	return total_size;
}


/*
 *	DNS�p�P�b�g�̉�́A���o
 */
int ExpandDNSPacket(char *recv_buf,		/*DNS�p�P�b�g*/
					int dns_id,			/*���ʔԍ�*/
					RES_RECORD *res)	/*���ʂ͂����ɓ���*/
{
	//16bit(2byte) , 8bit(1byte) span pointer
	u_short *us_ptr;
	char *strptr;

	int questions_num;		/*���⃌�R�[�h��*/
	int answer_num;			/*�񓚃��R�[�h��*/
	int authority_num;		/*���Ѓ��R�[�h��*/
	int additional_num;		/*�ǉ����R�[�h��*/
	
	us_ptr = (u_short *)recv_buf;
	/*���ʔԍ��̃`�F�b�N*/
	if(ntohs(us_ptr[0]) != dns_id){
		DebugPrint("Packet�̎��ʔԍ����A���M���̂��̂ƈႢ�܂��B");
		return -1;
	}
	/*�t���O�̃`�F�b�N*/
	if((ntohs(us_ptr[1]) & 0x000f) != 0){
		DebugPrint("�����p�P�b�g�ł͂���܂���B");
		return -1;
	}
	/*���␔�A�񓚐��A���А��A�ǉ�RR���̎擾*/
	questions_num = ntohs(us_ptr[2]);	//���␔
	answer_num = ntohs(us_ptr[3]);		//�񓚐�
	authority_num = ntohs(us_ptr[4]);	//���А�
	additional_num = ntohs(us_ptr[5]);	//�ǉ���

	//���╔���R�[�h�̊J�n�_
	strptr = (char *)&us_ptr[6];

	//���R�[�h����f�[�^���擾����
	ExpandDNSRecords(recv_buf,strptr,res,
		questions_num,answer_num,authority_num,additional_num);

	return 0;
}


/*
 *	DNS�T�[�o�[���瑗���Ă������R�[�h����͂���֐�
 */
int ExpandDNSRecords(char		*packet,		/*DNS�p�P�b�g*/
					 char		*pch,
					 RES_RECORD	*res,			/*���\�[�X���R�[�h�i�[�p*/
					 int		query,			/*���⃌�R�[�h��*/
					 int		answer,			/*�񓚃��R�[�h��*/
					 int		authority,		/*���Ѓ��R�[�h��*/
					 int		add)			/*�ǉ����R�[�h��*/
{
	int i;
	int cnt = 0;

	/*���╔���R�[�h����́A�擾����*/
	pch = SkipQuestionsRecord(packet,pch);

	/*�񓚕����R�[�h����́A�擾����*/
	for(i=0;i<answer;i++)
		pch = ExpandDNSResRecords(packet,pch,&res[cnt++]);
	
	/*���Е����R�[�h����́A�擾����*/
	for(i=0;i<authority;i++)
		pch = ExpandDNSResRecords(packet,pch,&res[cnt++]);

	/*�ǉ������R�[�h����́A�擾����*/
	for(i=0;i<add;i++)
		pch = ExpandDNSResRecords(packet,pch,&res[cnt++]);
	
	return 0;
}


/*
 *	���╔���X�L�b�v����
 */
char *SkipQuestionsRecord(char *packet,		/*�p�P�b�g�擪��*/
						  char *top_ptr)	/*���╔���R�[�h�̊J�n�_*/
{
	char buffer[256]="";
	u_short *us_ptr;	//unsigned short pointer.
	
	/*�Ɖ���΂�*/
	top_ptr = ExpandDNSLabel(packet,top_ptr,buffer);
	/*�Ɖ�N���X�A�^�C�v���΂�*/
	us_ptr = (u_short *)top_ptr;
		
	return (char *)&us_ptr[2];
}


/*
 *	�񓚕��A���Е��A�ǉ���񕔂̃��R�[�h�𒊏o����֐�
 */
char *ExpandDNSResRecords(char			*packet,
						  char			*top_ptr,
						  RES_RECORD	*res)
{
	u_short *us_ptr;

	/*�f�[�^������̂�FLAG��TRUE�ɂ���*/
	res->flag = TRUE;

	/*�Ɖ�x���̎擾*/
	top_ptr = ExpandDNSLabel(packet,top_ptr,res->query);
	
	/*�N���X�A�^�C�v*/
	us_ptr = (u_short *)top_ptr;
	res->dns_type = ntohs(us_ptr[0]);
	res->dns_class = ntohs(us_ptr[1]);
	/*us_ptr[2],[3]�́ATTL*/
	/*us_ptr[4]�́A���\�[�X�f�[�^�̒���*/
	/*us_ptr[5]�́A���\�[�X�f�[�^�J�n�_*/

	/*���\�[�X�f�[�^�̎擾*/
	top_ptr = ExpandDNSResData(packet,(char *)&us_ptr[5],res);

	return top_ptr;
}


/*
 *	���k���ꂽ���O��W�J����֐�
 */
char *ExpandDNSLabel(char *top,			/*DNSPacket�擪�ʒu*/
					 char *source,		/*���O�̈ʒu*/
					 char *result)		/*���ʂ��o�͂���o�b�t�@*/
{
	int len,i;
	char *temp;
	
	while((len = *source++) != 0){
		/*���k����Ă�����*/
		if((len & 0xC0) == 0xC0){
			temp = top + *source;
			ExpandDNSLabel(top,temp,result);
			return ++source;
		}
		for(i=0;i<len;i++){
			*result++ = *source++;
		}
		*result++ = '.';
	}
	*--result = '\0';
	return source;
}


/*
 *	���\�[�X�f�[�^�̎擾������
 */
char *ExpandDNSResData(char			*packet,	/*�p�P�b�g*/
					   char			*res_ptr,	/*���\�[�X�f�[�^�J�n�_*/
					   RES_RECORD	*res)		/*�^�C�v*/
{
	char buffer[MAX_BUFFER_SIZE]="";
	u_short *us_ptr = NULL;
	int *i_ptr = NULL;

	switch(res->dns_type){
	/*IP ADDRESS*/
	case DNS_TYPE_A:
		i_ptr = (int *)res_ptr;
		/*IP�A�h���X�̎擾*/
		res->ip_addr = *i_ptr++;
		return (char *)i_ptr;
	/*NAME SERVER*/
	case DNS_TYPE_NS:
		return ExpandDNSLabel(packet,res_ptr,res->domain);
	case DNS_TYPE_PTR:
		return ExpandDNSLabel(packet,res_ptr,res->domain);
	case DNS_TYPE_CNAME:
		return ExpandDNSLabel(packet,res_ptr,res->domain);
	/*MAIL EXCHANGER*/
	case DNS_TYPE_MX:
		us_ptr = (u_short *)res_ptr;
		/*�D��x�̎擾*/
		res->priority = ntohs(*us_ptr++);
		return ExpandDNSLabel(packet,(char *)us_ptr,res->domain);
	default:
		DebugPrint("���m�̌`���ł��B");
		return NULL;
	}
	return res_ptr;
}

/***	End of file		***/