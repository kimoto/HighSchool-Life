/*
 *	UDPヘッダー書き換えテストプログラム
 *	サーバー側。受信を待ち、受信したものを表示するだけのプログラム
 *
 *	引数についての詳細は、もうちょっと下の#define文にあります。	
 *	2004/08/27 : written by kimoto.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define ERROR_SYNTAX \
 "\n"\
 "引数は、[./udp_server IPAddress PortNumber\\n]ですよ。\n"\
 "\n"\
 " IPAddressには、どのアドレスで受信を待つかを入力して下さい。,\n"\
 " 基本的に、127.0.0.1の loopback address を設定します。(実験用なので)\n"\
 "\n"\
 "PortNumberは、どのポート番号でデータを待つかです。\n"\
 "他のプログラムのポート番号とかぶらないように注意して下さい。\n"\
 "\n"\
 "例:\n"\
 "./udp_server 127.0.0.1 5000"\
 "\n"

/*
 *	メイン関数
 */
int main(int argc,char **argv)
{
	int sock = 0;			//0で初期化しないと例外処理がうまく行かない。
	int fromlen;
	int port_number;
	char recv_buf[256];		//受信用バッファ
	struct sockaddr_in addr;

	//本来ならきちんと引数を解析しなければならない。
	//めんどくさいので省略,適当にやる
	if(argc != 3){
		puts(ERROR_SYNTAX);
		return -1;
	}

	//引数を解析して、Port番号を取得する
	port_number = atoi(argv[2]);	//Portは、第二引数

	//ソケット作成、定石? UDPのソケットを作り、バインドして待つ
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0){
		puts("socket error");
		return -1;
	}

	//sockaddr_in 構造体に情報を格納する
	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_number);
	addr.sin_addr.s_addr = inet_addr(argv[1]);	//第一引数は、そのままで問題無し!!

	//バインドする、Port番号を確保するってことだと思ってよし。
	//他のプログラムに使えなくする。自分専用。
	if(bind(sock,(struct sockaddr *)&addr,sizeof(struct sockaddr)) < 0){
		puts("bind error");
		goto error;
	}
	
	fromlen = sizeof(struct sockaddr);

	//データを受信する。データが来るまでずっと待つ。寂しい関数。
	if(recvfrom(sock,recv_buf,sizeof(recv_buf),0,(struct sockaddr *)&addr,&fromlen) < 0){
		puts("recvfrom error or not connection.");
		goto error;
	}

	printf("データを受信しました。\n%s\n",recv_buf);
	
	return 0;

//例外処理
error:
	if(sock != 0){
		close(sock);
	}
	return -1;
}	
