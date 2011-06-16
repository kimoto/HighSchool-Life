/*
 *	DNSクライアントプログラム
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
	u_short	dns_type;		/*タイプ*/
	u_short	dns_class;		/*クラス*/
	int		priority;		/*優先度*/
	int		ip_addr;		/*IPアドレス*/
	char	domain[256];	/*ドメイン名*/
	char	query[256];		/*照会ラベル*/
	BOOL	flag;			/*データが入っているか*/
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
 *	メイン関数
 */
int main(int argc,char *argv[])
{
	char packet[MAX_BUFFER_SIZE]="";
	RES_RECORD res[20];

	/*DNSデータの受信*/
	GetDNSData("192.168.0.1","yahoo.co.jp",111,DNS_TYPE_MX,packet,sizeof(packet));
	/*初期化*/
	ZeroMemory(&res,sizeof(res));
	/*受信したデータの解析*/
	ExpandDNSPacket(packet,111,res);

	int i = 0;
	printf("***********************************\n");
	printf("SERVER \n");
	//MXレコードのみを結果として表示する
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
 *	DNSデータを受信するプログラム
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

	/*DNS用ソケットの作成(UDP)*/
	sock = socket(PF_INET,SOCK_DGRAM,0);
	if(sock == INVALID_SOCKET){
		ErrorMessage("socket error");
	}
	/*DNSパケットの作成*/
	send_size = MakeDNSPacket(dns_id,domain,
		send_buf,DNS_RD_RECURSION,dns_type,DNS_CLASS_IN);
	/*DNSパケットの送信のための設定*/
	sockaddr_in.sin_addr.S_un.S_addr = inet_addr(dns_server);//DNSServerAddress
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(UDP_DNS_PORT);
	/*DNSパケットの送信*/
	return_val = sendto(sock,send_buf,send_size,0,
		(SOCKADDR *)&sockaddr_in,sizeof(sockaddr_in));
	if(return_val == SOCKET_ERROR){
		ErrorMessage("sendto error");
	}
	/*DNSパケットの受信*/
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
 *	DNSパケットを作成する関数
 *	ほとんどすべての形式に対応する。
 *
 *	2004/06/27 program by kimoto
 */
size_t MakeDNSPacket(u_short	dns_id,		/*パケットID番号*/
					 char		*domain, 	/*ドメイン名*/
					 char		*send_buf,	/*送信バッファ*/
					 u_short	dns_flag,	/*処理・応答コードのフラグ*/
					 u_short	dns_type,	/*DNSに要求するレコードのタイプ(mx,a等)*/
					 u_short	dns_class)	/*DNSのクラス*/
{
	u_short *us_ptr;
	size_t total_size;
	
	/*16bit(2byte)でアクセスするためのポインタ*/
	us_ptr = (u_short *)send_buf;

	us_ptr[0] = htons(dns_id);		/*識別番号*/
	us_ptr[1] = htons(dns_flag);	/*処理・応答コード(再帰的問い合わせ)*/
	us_ptr[2] = htons(1);			/*質問部の要素数*/
	us_ptr[3] = htons(0);			/*回答部の要素数*/
	us_ptr[4] = htons(0);			/*オーソリティ部の要素数*/
	us_ptr[5] = htons(0);			/*追加情報部の要素数*/
	total_size = 12;
	
	/*質問部を作成*/
	total_size += MakeDNSQuery((char *)&us_ptr[6],domain,dns_type,dns_class);
	
	return total_size;
}


/*
 *	DNSパケットの質問レコードを作成する関数
 */
size_t MakeDNSQuery(char *packet,
					char *domain,
					u_short dns_type,
					u_short dns_class)
{
	int total_size = 0;
	u_short *us_ptr;
	
	/*照会ラベルを作成*/
	total_size = MakeDNSQueryLabel(domain,packet);
	packet += total_size;
	
	/*照会タイプ、クラスを作成*/
	us_ptr = (u_short *)packet;
	us_ptr[0] = htons(dns_type);
	us_ptr[1] = htons(dns_class);
	
	/*照会ラベルは2byte,照会クラスも2byte*/
	return total_size + 2 + 2;
}


/*
 *	指定されたドメイン名から照会用ラベルを作成する関数
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
		/*token_lenを出力する*/
		*result_ptr++ = token_len;
		total_size++;
		/*tokenを出力する*/
		for(i=0;i<token_len;i++){
			*result_ptr++ = *token++;
		}
		token = strtok(NULL,".");
	}
	/*末尾は、0*/
	*result_ptr++ = 0;
	total_size++;

	*result_ptr++ = '\0';
	
	return total_size;
}


/*
 *	DNSパケットの解析、抽出
 */
int ExpandDNSPacket(char *recv_buf,		/*DNSパケット*/
					int dns_id,			/*識別番号*/
					RES_RECORD *res)	/*結果はここに入る*/
{
	//16bit(2byte) , 8bit(1byte) span pointer
	u_short *us_ptr;
	char *strptr;

	int questions_num;		/*質問レコード数*/
	int answer_num;			/*回答レコード数*/
	int authority_num;		/*権威レコード数*/
	int additional_num;		/*追加レコード数*/
	
	us_ptr = (u_short *)recv_buf;
	/*識別番号のチェック*/
	if(ntohs(us_ptr[0]) != dns_id){
		DebugPrint("Packetの識別番号が、送信時のものと違います。");
		return -1;
	}
	/*フラグのチェック*/
	if((ntohs(us_ptr[1]) & 0x000f) != 0){
		DebugPrint("応答パケットではありません。");
		return -1;
	}
	/*質問数、回答数、権威数、追加RR数の取得*/
	questions_num = ntohs(us_ptr[2]);	//質問数
	answer_num = ntohs(us_ptr[3]);		//回答数
	authority_num = ntohs(us_ptr[4]);	//権威数
	additional_num = ntohs(us_ptr[5]);	//追加数

	//質問部レコードの開始点
	strptr = (char *)&us_ptr[6];

	//レコードからデータを取得する
	ExpandDNSRecords(recv_buf,strptr,res,
		questions_num,answer_num,authority_num,additional_num);

	return 0;
}


/*
 *	DNSサーバーから送られてきたレコードを解析する関数
 */
int ExpandDNSRecords(char		*packet,		/*DNSパケット*/
					 char		*pch,
					 RES_RECORD	*res,			/*リソースレコード格納用*/
					 int		query,			/*質問レコード数*/
					 int		answer,			/*回答レコード数*/
					 int		authority,		/*権威レコード数*/
					 int		add)			/*追加レコード数*/
{
	int i;
	int cnt = 0;

	/*質問部レコードを解析、取得する*/
	pch = SkipQuestionsRecord(packet,pch);

	/*回答部レコードを解析、取得する*/
	for(i=0;i<answer;i++)
		pch = ExpandDNSResRecords(packet,pch,&res[cnt++]);
	
	/*権威部レコードを解析、取得する*/
	for(i=0;i<authority;i++)
		pch = ExpandDNSResRecords(packet,pch,&res[cnt++]);

	/*追加部レコードを解析、取得する*/
	for(i=0;i<add;i++)
		pch = ExpandDNSResRecords(packet,pch,&res[cnt++]);
	
	return 0;
}


/*
 *	質問部をスキップする
 */
char *SkipQuestionsRecord(char *packet,		/*パケット先頭部*/
						  char *top_ptr)	/*質問部レコードの開始点*/
{
	char buffer[256]="";
	u_short *us_ptr;	//unsigned short pointer.
	
	/*照会名を飛ばす*/
	top_ptr = ExpandDNSLabel(packet,top_ptr,buffer);
	/*照会クラス、タイプを飛ばす*/
	us_ptr = (u_short *)top_ptr;
		
	return (char *)&us_ptr[2];
}


/*
 *	回答部、権威部、追加情報部のレコードを抽出する関数
 */
char *ExpandDNSResRecords(char			*packet,
						  char			*top_ptr,
						  RES_RECORD	*res)
{
	u_short *us_ptr;

	/*データが入るのでFLAGをTRUEにする*/
	res->flag = TRUE;

	/*照会ラベルの取得*/
	top_ptr = ExpandDNSLabel(packet,top_ptr,res->query);
	
	/*クラス、タイプ*/
	us_ptr = (u_short *)top_ptr;
	res->dns_type = ntohs(us_ptr[0]);
	res->dns_class = ntohs(us_ptr[1]);
	/*us_ptr[2],[3]は、TTL*/
	/*us_ptr[4]は、リソースデータの長さ*/
	/*us_ptr[5]は、リソースデータ開始点*/

	/*リソースデータの取得*/
	top_ptr = ExpandDNSResData(packet,(char *)&us_ptr[5],res);

	return top_ptr;
}


/*
 *	圧縮された名前を展開する関数
 */
char *ExpandDNSLabel(char *top,			/*DNSPacket先頭位置*/
					 char *source,		/*名前の位置*/
					 char *result)		/*結果を出力するバッファ*/
{
	int len,i;
	char *temp;
	
	while((len = *source++) != 0){
		/*圧縮されていた時*/
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
 *	リソースデータの取得をする
 */
char *ExpandDNSResData(char			*packet,	/*パケット*/
					   char			*res_ptr,	/*リソースデータ開始点*/
					   RES_RECORD	*res)		/*タイプ*/
{
	char buffer[MAX_BUFFER_SIZE]="";
	u_short *us_ptr = NULL;
	int *i_ptr = NULL;

	switch(res->dns_type){
	/*IP ADDRESS*/
	case DNS_TYPE_A:
		i_ptr = (int *)res_ptr;
		/*IPアドレスの取得*/
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
		/*優先度の取得*/
		res->priority = ntohs(*us_ptr++);
		return ExpandDNSLabel(packet,(char *)us_ptr,res->domain);
	default:
		DebugPrint("未知の形式です。");
		return NULL;
	}
	return res_ptr;
}

/***	End of file		***/