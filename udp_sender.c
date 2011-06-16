/*
 *	UDPヘッダー書き換えプログラム
 *
 *	引数は、./sender SrcIPAddr DestIPAddr Port Data\n
 *	左から、送信元IP 送信先IP ポート番号 送信したい文字列データ
 *
 *	RAWソケットを使うのでrootが使用してください。
 *	2004/08/27 : written by kimoto.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>

//ここ増やすとたくさんの文字列データ送れます。
#define STR_MAXSIZE 256

#define ERROR_SYNTAX \
 "\n"\
 "引数が違います。\n"\
 "\n"\
 "引数は、./sender SrcIPAddr DestIPAddr Port Data\\nです。\n"\
 "SrcIPAddrは、送信元IPアドレスを入力して下さい。\n"\
 "DestIPAddrは、送信先IPアドレスを入力して下さい。\n"\
 "Portは、サーバー側で設定したポート番号を入力して下さい。\n"\
 "Dataは、送りたい文字列データを入力して下さい。\n"\
 "\n"\
 "このプログラムは、Rawソケットを使用するのでroot権限を持つユーザーが実行して下さい。\n"\
 "\n"

/*
 *	Rawソケットで送信するために必要なデータ
 */
struct SendPacket{
	struct iphdr ip;
	struct udphdr udp;
	char data[STR_MAXSIZE];
};

/*
 *	メイン関数
 */
int main(int argc,char **argv)
{
	struct SendPacket send;
	int data_len;
	int packet_len;
	int sock;
	int port_number;
	struct sockaddr_in addr;

	//引数分析、いつものように適当
	if(argc != 5){
		puts(ERROR_SYNTAX);
		return -1;
	}

	//引数から、ポート番号を取得、ポート番号だけ、変換が必要。
	port_number = atoi(argv[3]);	
	
	//送信元、送信先の情報を(sendto,recvfromで使う)
	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);	

	//送るデータを作成する
	//とりあえず、送るデータの長さも調べとく
	strcpy(send.data,argv[4]);
	data_len = strlen(send.data);

	//IPヘッダの設定
	send.ip.version = 4;	//IPv4だからversionは、4.
	send.ip.ihl = 5;
	send.ip.ttl = 255;	//何回ルータ?通れるか.
	send.ip.protocol = 17;	//UDPは、17.
	send.ip.saddr = inet_addr(argv[1]);
	send.ip.daddr = inet_addr(argv[2]);
	
	//UDPヘッダの設定
	send.udp.source = htons(port_number);
	send.udp.dest = htons(port_number);
	send.udp.len = htons(data_len + 8);	//UDPヘッダの長さ

	//パケットの長さを調べる
	packet_len = sizeof(send.ip) + sizeof(send.udp) + data_len;
	printf("packet_len %d\n",packet_len);

	//ソケットを作る、定石
	sock = socket(AF_INET,SOCK_RAW,IPPROTO_RAW);
	if(sock < 0){
		puts("socket error");
		return -1;
	}

	//組み立てたパケットを送信する
	if(sendto(sock,&send,packet_len,0,(struct sockaddr *)&addr,sizeof(struct sockaddr)) < 0){
		puts("sendto error");
		goto error;
	}

	//あとは、自爆してく - See You Again!!
	puts("送信完了 - See You Again!!");

	close(sock);

	return 0;

//例外処理
error:
	if(sock != 0){
		close(sock);	
	}
	return -1;
}
