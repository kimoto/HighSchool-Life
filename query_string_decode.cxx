#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 16進数文字を
// 数字に変換する補助関数
// 16進数でない文字だった場合 負の値 を返す
inline
int AtoH(const char hex)
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
int HextoInt(const char *hex,int len)
{
	int value = 0;
	if( len <= 0 ){
		return -1;
	}

	while(len-- > 0){
		// null check
		if(*hex == '\0'){
			break;
		}
		// error check
		int i = AtoH(*hex++);
		if(i < 0){
			return - 1;
		}
		value += i << 4 * len;
	}
	return value;
}

// デコード関数
char *decodeFormData(const char *str)
{
	int len = strlen(str);
	char *buffer = new char[len + 1];
	if(!buffer){
		return NULL;
	}

	char *result = buffer;
	while(*str != '\0'){
		if(*str == '%'){
			str++;
			int n = HextoInt(str,2);
			if(n < 0){
				return NULL;
			}
			str+=2;
			*result++ = n;
		}else if(*str == '+'){
			*result++ = ' ';
			str++;
		}else{
			*result++ = *str++;
		}
	}
	*result = '\0';
	return buffer;
}

// test code
int main()
{
	// case 1
	char *data = "%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A";
	char *buffer = decodeFormData(data);
	puts(buffer);
	delete buffer;

	// case 2
	buffer = decodeFormData("this+is+test.");
	puts(buffer);
	delete buffer;

	// case 3
	buffer = decodeFormData("this+%E3%81%82+is+test.+[]#*-^$#&%");
	puts(buffer);
	delete buffer;
	return 0;
}

// created by kimoto.
