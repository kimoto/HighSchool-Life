/*
 *	文字コード変換に関連する関数
 *	Shift-JisからJisコードへの変換にしか対応していない（メール送信では十分）
 *
 *	2004/07/21 : by kimoto
 */
#include <windows.h>
#include <mbstring.h>
#include "convert.h"

static int convert_SJisToJis(const char *src,char *dest);

/*
 *	SJisからJISに変換する、戻り値は、変換後のサイズ
 */
extern size_t SJisToJis(const char *src,char *dest)
{
	if(convert_SJisToJis(src,dest) != 0){
		return -1;
	}
	return lstrlen(dest);
}

/*
 *	Shift-JISからJISに変換する
 *	成功したら戻り値は0
 *	末尾に'\0'を代入する
 */
static int convert_SJisToJis(const char *src,char *dest)
{
	int shift_flag = FALSE;

	/*まず、普通のアルファベットの時は、普通に出力*/
	while(*src != '\0'){
		/*普通のASCIIの時は普通に出力*/
		if(_ismbblead(*src) == 0){
			*dest++ = *src++;
			continue;
		}
		/*SJISを見つけたら開始コードを出力*/
		if(_ismbblead(*src) != 0){
			/*１つ前の文字が、SJISじゃなかったら*/
			if(shift_flag == FALSE){
				*dest++ = 0x1b;
				*dest++ = 0x24;
				*dest++ = 0x42;
			}
			*dest++ = HIBYTE(_mbcjmstojis(MAKEWORD(*(src+1),*src)));
			*dest++ = LOBYTE(_mbcjmstojis(MAKEWORD(*(src+1),*src)));
			/*次の文字がまだ、SJISだったらまだ終了コードをつけない。*/
			/*逆に言えば、次の文字がJISではなかったら終了コードをつける*/
			src+=2;	/*次の文字へ*/
			shift_flag = TRUE;
			if(!(_ismbblead(*src) != 0)){
				*dest++ = 0x1b;
				*dest++ = 0x28;
				*dest++ = 0x42;
				shift_flag = FALSE;
				continue;
			}
		}
	}
	/*末尾にNULL文字を付ける*/
	*dest = '\0';
	return 0;
}