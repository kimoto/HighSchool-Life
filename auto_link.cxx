#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define LINK_FMT "<a href=\"%s\">%s</a>"

bool isurlchar(char c)
{
	char *table = ".:/";

	if(isalnum(c)){
		return true;
	}
	for(char *url=table; *url!='\0'; url++){
		if(*url == c){
			return true;
		}
	}
	return false;
}

char *auto_link(char *url)
{
	char *result = NULL;

	// check urlchar length
	int len = strlen(url);
	int url_len = 0;
	for(char *str=url; *str!='\0'; str++){
		if(isurlchar(*str)){
			url_len++;
		}else{
			break;
		}
	}

	// urlchar strings
	char *urlstr = new char[url_len + 1];
	strncpy(urlstr,url,url_len);
	urlstr[url_len] = '\0';

	// result buffer
	int size = len*2 + strlen(LINK_FMT);
	result = new char[size + 1];
	result[size] = '\0';

	// last
	snprintf(result,size,LINK_FMT,urlstr,urlstr);
	delete [] urlstr;
	return result;
}

int main()
{
	puts( auto_link("http://www.yahoo.co.jp/ akdkd") );
	return 0;
}
