/*
 *	16進数変換コード
 *	Created by kimoto.
 */

#include <stdio.h>
#include <string.h>

// 16進数文字を
// 数字に変換する補助関数
// 16進数でない文字だった場合 負の値 を返す
inline
int AtoH(char hex)
{
	if(0 <= hex && hex <= 9){
		return hex;
	}else if('0' <= hex && hex <= '9'){
		return hex - '0';
	}else if('a' <= hex && hex <= 'f'){
		return hex - 'a' + 10;
	}else if('A' <= hex && hex <= 'F'){
		return hex - 'A' + 10;
	}else{
		return -1;
	}
	return 0;
}

// 16進数文字列を変換して数字にする
// 変換できない文字があったとき -1 を返す
int HextoInt(char *hex)
{
	int value = 0;
	int len = strlen(hex);

	while(len-- > 0){
		// error check
		int i = AtoH(*hex++);
		if(i < 0){
			return - 1;
		}
		value += i << 4 * len;
	}
	return value;
}

// test code
int main()
{
	char *hex = "e1";
	int n = 0;

	// まずい overflow してた!!
	n = HextoInt( hex );
	printf("hex[%s] => int[%d];\n", hex, n);
	return 0;
}

// result
// hex[e1] = int[225];
// 							created by kimoto.
