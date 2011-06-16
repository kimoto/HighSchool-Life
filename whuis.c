/*
 *	char code : UTF-8
 *	whois コマンドのぱくり whuisコマンド
 *
 *	comments:
 *	俺、引数解析が嫌いなんでその辺は全然やってません。
 *
 *	editor: vim
 *	2004/08/26 : written by kimoto.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>	//socket etc...
#include <unistd.h>	//close etc...
#include <netdb.h>

#define WHOIS_HOST_SERVER "internic.net"
#define TCP_WHOIS_PORT 43
/*
 *	メイン関数
 *	第一引数は、調べたいドメイン名 ( argv[1] )
 */
int main(int argc,char **argv)
{
	int sock;
	int res;
	int fromlen;
	struct hostent *phost;
	struct sockaddr_in inet_addr;
	char send_buf[256];
	char recv_buf[1024];

	/*ソケット作ってます。*/
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0){
		puts("socket error!!");	
		return -1;
	}
	/*
	 * whoisを管理してるサーバーの
	 * IPアドレスを取得
	 */
	phost = gethostbyname(WHOIS_HOST_SERVER);
	if(phost == NULL){
		puts("gethostbyname error!!");
		return -1;
	}

	/*
	 * whoisを管理してるサーバーに
	 * TCP - 43番Portでアクセスする
	 */
	memset(&inet_addr,0,sizeof(inet_addr));
	inet_addr.sin_family = AF_INET;
	inet_addr.sin_port = htons(TCP_WHOIS_PORT);
	memcpy(&inet_addr.sin_addr,phost->h_addr,phost->h_length);
	/* Connect!! */
	//if(connect(sock,(struct sockaddr *)&inet_addr,
	//			sizeof(struct sockaddr)) == -1){
	//	puts("connect error!!");
	//	return -1;
	//}
	//puts("connect success!!");

	/*サーバーに送るデータを作る*/
	res = snprintf(send_buf,sizeof(send_buf),"%s\r\n",argv[1]);
	if(res > sizeof(send_buf)){
		puts("snprintf error!!");	
		return -1;
	}

	printf("確認、確認、%s\n",send_buf);
	/*めんどくさいからそのまま質問をwhoisサーバーQuery!!*/
	//res = send(sock,send_buf,strlen(send_buf),0);
	res = sendto(sock,send_buf,strlen(send_buf),0,
		(struct sockaddr *)&inet_addr,sizeof(struct sockaddr));
	if(res == -1){
		puts("send error!!");
		return -1;
	}
	printf("送信したデータ量は、%dでしたよ。\n",res);

	/*最後に応答を受信して表示する*/
	//res = recv(sock,recv_buf,strlen(recv_buf),0);

	fromlen = sizeof(struct sockaddr);
	res = recvfrom(sock,recv_buf,sizeof(recv_buf),0,
		(struct sockaddr *)&inet_addr,&fromlen);
	if(res == -1){
		puts("recv error!!");
		return -1;
	}
	printf("応答したデータ量は、%dでしたよ。\n",res);
	/*表示*/
	puts(recv_buf);

	puts("通信が無事に終了しました。");
	printf("ちなみに、引数は、%s %s\n",argv[0],argv[1]);
	close(sock);
	return 0;
}
