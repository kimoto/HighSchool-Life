/*
*	DNSサーバーからMXレコードを取得するための関数群
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

/*	照会タイプ	*/
#define DNS_TYPE_A		 1
#define DNS_TYPE_NS		 2
#define DNS_TYPE_CNAME	 5
#define DNS_TYPE_PTR	12
#define DNS_TYPE_MX		15
#define DNS_TYPE_AAAA	28
#define DNS_TYPE_ANY	255

/*	照会クラス	*/
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
#define ERROR_ID_NUMBER		-1	/*識別番号が違う*/

/*	FLAG ERROR */
#define RCODE_ERROR_NONE	0	/*エラーなし*/
#define RCODE_ERROR_FORMAT	1	/*構文エラー*/
#define RCODE_ERROR_SERVER	2	/*サーバーのエラー*/
#define RCODE_ERROR_NAME	3	/*ネームエラー*/
#define RCODE_ERROR_XXXX	4	/*未実装*/
#define RCODE_ERROR_REFUSAL	5	/*拒否された*/
#define RCODE_ERROR			6	/*未知のエラー*/

/* other */
static int SkipQuery(char **src,int n);
#define ERROR_NULL_PTR	-1
static int GetMXFromAnswer(char *packet,char *pch,int answer_num,MXRECORD *mx);
#define ERROR_NOT_MX	-1
static char *ExpandDNSLabel(char *top,char *source,char *result);
static int CheckDNSFlag(int flag);

/*
*	MXレコードを取得する関数
*	グローバル関数は、これだけ
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
	/*DNSサーバーへの接続の準備*/
	sockaddr_in.sin_addr.S_un.S_addr = inet_addr(dns_server_addr);
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(UDP_DNS_PORT);
	
	/*DNSパケットをサーバーに送信する*/
	if(SOCKET_ERROR == sendto(sock,send_buf,packet_size,0,
		(SOCKADDR *)&sockaddr_in,sizeof(sockaddr_in))){
		return GETMX_ERROR_SENDTO;
	}
	fromlen = sizeof(sockaddr_in);
	/*サーバーからの応答を受信する*/
	if(SOCKET_ERROR == recvfrom(sock,recv_buf,sizeof(recv_buf),0,
		(SOCKADDR *)&sockaddr_in,&fromlen)){
		return GETMX_ERROR_RECV;
	}
	/*受信したデータを解析、取得する*/
	ExpandDNSPacket(recv_buf,111,mx);
	
	closesocket(sock);
	WSACleanup();
	return GETMX_ERROR_SUCCESS;
}


/*-------------------------------------------------------
* 関数名 : MakeDNSPacket
*-------------------------------------------------------
* 説明
*	DNSパケットを作成する関数
*	ほとんどすべての形式に対応する
*
* 引数
*	dns_id		:	識別番号(パケットID番号)
*	domain		:	照会したいドメイン名を指定する
*	send_buf	:	送信バッファを指定
*	dns_flag	:	処理・応答コードのフラグ
*	dns_type	:	DNSに要求するリソースレコードのタイプ(MX,A,AAAA,NS等)
*	dns_class	:	DNSのクラス(inter net等)
* 戻り値
*	作成したDNSパケットの総サイズ
*-------------------------------------------------------
* 履歴
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
	/*16bit(2byte)単位でアクセスするためのポインタ*/
	u_short *us_ptr = (u_short*)send_buf;
	
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

/*-------------------------------------------------------
* 関数名 : ExpandDNSPacket
*-------------------------------------------------------
* 説明
* 指定されたDNSパケットからMXレコードの情報を
* 抽出する関数
*
* 引数
*	packet : DNSパケット
*	dns_id : DNSパケット送信時に付けたユニークな番号
*	mx     : 抽出したデータを格納する構造体
* 戻り値
*	ERROR_SUCCESS		 0 : 関数が成功した時
*	ERROR_ID_NUMBER		-1 : 識別番号に異常があった
*	error_code	:	フラグのエラーをそのまま返す
*-------------------------------------------------------
* 履歴
* 2004/07/05 : program by kimoto
*-------------------------------------------------------
*/
static int ExpandDNSPacket(char *packet,
						   int	dns_id,
						   MXRECORD *mx)
{
	/*2byte,1byte単位でアクセスするためのポインタ*/
	u_short *us_ptr;
	char *pch;
	
	int questions_num;			/*質問レコード数*/
	int answer_num;				/*回答レコード数*/
	int authority_num;			/*権威レコード数*/
	int additional_num;			/*追加レコード数*/
	
	int error_code;				/*フラグのエラーコード*/
	
	us_ptr = (u_short *)packet;
	/*識別番号のチェック*/
	if(ntohs(us_ptr[0]) != dns_id){
		DebugPrint("Packetの識別番号が、送信時のものと違います。");
		return ERROR_ID_NUMBER;
	}
	/*フラグのチェック*/
	error_code = CheckDNSFlag(ntohs(us_ptr[1]));
	if(error_code != RCODE_ERROR_NONE){
		return error_code;
	}
	DebugPrint("正常なパケットです。");
	
	/*質問数、回答数、権威数、追加RR数の取得*/
	questions_num = ntohs(us_ptr[2]);
	answer_num = ntohs(us_ptr[3]);
	authority_num = ntohs(us_ptr[4]);
	additional_num = ntohs(us_ptr[5]);
	
	/*質問部レコードの開始点*/
	pch = (char *)&us_ptr[6];
	
	/*質問部レコードの読み飛ばし*/
	SkipQuery(&pch,questions_num);
	
	/*回答部レコードの取得*/
	GetMXFromAnswer(packet,pch,answer_num,mx);
	
	//権威部レコード、追加情報部レコードは、使わない。
	
	return ERROR_SUCCESS;
}


/*-------------------------------------------------------
* 関数名 : CheckDNSFlag
*-------------------------------------------------------
* 説明
*	DNSパケットのフラグを調べる関数
*
* 引数
*	flag	:	サーバーから届いたデータ
* 戻り値
*	error_code	:	取得したエラーコードを返す
*-------------------------------------------------------
* 履歴
*	2004/07/05 : program by kimoto
*-------------------------------------------------------
*/
static int CheckDNSFlag(int flag)
{
	return (flag & CHECK_RCODE_ERROR);
}

/*-------------------------------------------------------
* 関数名 : SkipQuery
*-------------------------------------------------------
* 説明
*	指定された位置から始まる質問部レコードをスキップする
*
* 引数
*	src	:	質問部レコードの先頭位置を指定
*	n	:	質問部レコードの個数を指定
* 戻り値
*	ERROR_SUCCESS	 0 : 関数が成功した時
*	ERROR_NULL		-1 : 引数srcがNULLを指していたとき
*-------------------------------------------------------
* 履歴
*	2004/07/05 : program by kimoto
*-------------------------------------------------------
*/
static int SkipQuery(char **src,int	n)
{
	int len;
	
	if(*src == NULL){
		return ERROR_NULL_PTR;
	}
	
	/*質問部レコードの個数分ループ*/
	while(n-- > 0){
		while((len=**src) != 0){
			*src = *src + 1;
			*src = *src + len;	
		}
		/*末尾の0の分進める*/
		*src = *src + 1;
		/*QTYPE + QCLASS = 4byte*/
		*src = *src + 4;
	}
	return ERROR_SUCCESS;
}


/*-------------------------------------------------------
* 関数名 : GetMXFromAnswer
*-------------------------------------------------------
* 説明
*	回答部レコードから、MXリソースレコードの
*	ドメイン名(domain)、優先度(priority)を取得する関数
*
*	us_ptr[0]は、タイプ
*	us_ptr[1]は、クラス
*	us_ptr[2],[3]は、生存時間(TTL)
*	us_ptr[4]は、リソースデータの長さ
*	us_ptr[5]は、MXレコードの場合、優先度
*	us_ptr[6]は、MXレコードの場合、ドメイン名
*
* 引数
*	packet	:	受信したDNSパケットの先頭位置
*	pch		:	回答部レコードの先頭位置
*	answer	:	回答部レコードの個数
*	mx		:	取得したデータを格納するための構造体
*
* 戻り値
*	ERROR_SUCCESS		 0 : 関数が成功した時
*	ERROR_NOT_MX		-1 : MXレコードではなかった時
*-------------------------------------------------------
* 履歴
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
		/*照会ラベルの取得*/
		pch = ExpandDNSLabel(packet,pch,buffer);
		DebugPrint(buffer);
		
		/*このレコードのタイプが、MXレコードか調べる*/
		us_ptr = (u_short *)pch;
		if(ntohs(us_ptr[0]) != DNS_TYPE_MX){
			return ERROR_NOT_MX;
		}
		
		/*優先度を取得*/
		mx->priority = ntohs(us_ptr[5]);
		
		/*ドメイン名を取得*/
		pch = ExpandDNSLabel(packet,(char *)&us_ptr[6],mx->domain);
		
		/*データを入れたので、フラグを立てる*/
		mx->flag = TRUE;
		mx++;
	}
	
	/*データ末尾を表すため、FALSEを格納*/
	mx->flag = FALSE;
	
	return ERROR_SUCCESS;
}


/*-------------------------------------------------------
* 関数名 : ExpandDNSLabel
*-------------------------------------------------------
* 説明
*	圧縮されたラベルを展開する関数
*
* 引数
*	packet	:	受信したDNSPacketの先頭位置
*	label	:	展開したいラベルの先頭位置
*	result	:	結果を出力するバッファ
* 戻り値
*	展開した分進めたDNSPacketへのポインタ
*-------------------------------------------------------
* 履歴
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
		/*圧縮されていた時*/
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
* 関数名 : MakeDNSQuery
*-------------------------------------------------------
* 説明
*	DNSパケットの質問部レコードを作成する関数
*
* 引数
*	packet		:	DNSパケットの先頭位置を指定する
*	domain		:	照会したいドメイン名を指定する
*	dns_type	:	照会したいタイプを指定する(MX,A,AAAA,NS等)
*	dns_class	:	照会したいクラスを指定する(internet等)
* 戻り値
*	質問部レコードの総サイズを返す
*-------------------------------------------------------
* 履歴
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
	
	/*照会ラベルを作成*/
	total_size = MakeDNSQueryLabel(domain,packet);
	packet += total_size;
	
	/*照会タイプ、クラスを作成*/
	us_ptr = (u_short *)packet;
	us_ptr[0] = htons(dns_type);
	us_ptr[1] = htons(dns_class);
	
	/*QTYPE = 2byte,QCLASS = 2byte*/
	return total_size + 2 + 2;
}


/*-------------------------------------------------------
* 関数名 : MakeDNSQueryLabel
*-------------------------------------------------------
* 説明
*	指定されたドメイン名から照会用のラベルの作成
*
* 引数
*	src	:	ドメイン名を指定する
*	result	:	結果を出力するバッファを指定する
*
* 戻り値
*	作成した照会ラベルのサイズを返す
*-------------------------------------------------------
* 履歴
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
		/*.が見つかった時*/
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
	/*末尾の0と、NULL文字*/
	*dest++ = 0;
	*dest = '\0';
	
	/*strlenの結果に、0の分を付け足す*/
	return strlen(dest_top) + 1;
}
