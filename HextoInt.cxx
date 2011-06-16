/*
 *	16$B?J?tJQ49%3!<%I(B
 *	Created by kimoto.
 */

#include <stdio.h>
#include <string.h>

// 16$B?J?tJ8;z$r(B
// $B?t;z$KJQ49$9$kJd=u4X?t(B
// 16$B?J?t$G$J$$J8;z$@$C$?>l9g(B $BIi$NCM(B $B$rJV$9(B
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

// 16$B?J?tJ8;zNs$rJQ49$7$F?t;z$K$9$k(B
// $BJQ49$G$-$J$$J8;z$,$"$C$?$H$-(B -1 $B$rJV$9(B
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

	// $B$^$:$$(B overflow $B$7$F$?(B!!
	n = HextoInt( hex );
	printf("hex[%s] => int[%d];\n", hex, n);
	return 0;
}

// result
// hex[e1] = int[225];
// 							created by kimoto.
