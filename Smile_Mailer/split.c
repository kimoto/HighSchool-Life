/*
 *	文字列操作で、取り出したり変換したりといった関数
 */
#include <string.h>
#include "split.h"

/*
 *	メールアドレスからドメイン名を切り出す
 *	成功したら0を返す
 */
extern int SplitDomainName(char *address,char *dest)
{
	if((address == NULL) || (dest == NULL)){
		return -1;
	}
	
	while(*address != '\0'){
		if(*address == '@'){
			address++;
			strcpy(dest,address);
			break;
		}
		address++;
	}
	return 0;
}


/*
 *	フルパス名から直前のディレクトリ名だけにする
 */
extern int SplitDirectoryName(char *filepath,char *result)
{
	int len = strlen(filepath);

	while(len-- > 0){
		if(filepath[len] == '\\'){
			//filepath[len + 1] = '\0';
			strncpy(result,filepath,len);
			return 0;
		}
	}
	return -1;
}
