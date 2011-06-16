#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

char *assignBuffer(const char *str,size_t len)
{
	char *buffer = new char[len + 1];
	strncpy(buffer,str,len);
	buffer[len] = '\0';
	return buffer;
}

char *appendBuffer(char *&buffer,const char *str)
{
	if(buffer == NULL){
		buffer = assignBuffer(str,strlen(str));
	}else{
		int len = strlen(buffer);
		len += strlen(str);

		char *temp = assignBuffer(buffer,len);
		strncpy(temp,buffer,len);
		strncat(temp,str,len);
		delete buffer;

		buffer = assignBuffer(temp,len);
		delete temp;
	}
	return buffer;
}

char *dynamicRead(FILE *fp)
{
	char *buffer = NULL;
	char block[1024]="";

	while(fgets(block,sizeof(block),fp) != NULL){
		appendBuffer(buffer,block);
	}

	return buffer;
}

// test routine
int main()
{
	FILE *stream;
	if((stream = fopen("test.txt","r")) == NULL){
		return 0;
	}

	// dynamic read && result show!!
	char *buffer = dynamicRead(stream);
	printf(buffer);
	delete buffer;

	fclose(stream);
	return 0;
}

// created by kimoto
