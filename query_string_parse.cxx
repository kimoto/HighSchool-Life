#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// parse formdata
typedef struct formdata{
	char *key;
	char *value;
	struct formdata *next;
}FORMDATA;

char *alloc_buffer(char *str,size_t size)
{
	char *buffer = new char[size + 1];
	strncpy(buffer,str,size);
	buffer[size] = '\0';

	return buffer;
}

char *strsplit(char *str,char c,char *&s1,char *&s2)
{
	for(char *ptr=str; *ptr!='\0'; ptr++){
		if(*ptr == c){
			s1 = alloc_buffer(str,ptr - str);
			ptr++;

			if(*ptr == '\0'){
				return NULL;
			}
			int len = strlen(ptr);
			s2 = alloc_buffer(ptr,len);
			return ptr;
		}
	}
	return NULL;
}

FORMDATA *add_formdata(FORMDATA *data,char *key,char *value)
{
	FORMDATA *temp = new FORMDATA;
	temp->key = alloc_buffer(key,strlen(key));
	temp->value = alloc_buffer(value,strlen(value));
	temp->next = NULL;

	if(data == NULL){
		data = temp;
	}else{
		FORMDATA *tail = data;
		while(tail->next != NULL){
			tail = tail->next;
		}
		tail->next = temp;
	}

	return data;
}

FORMDATA *parse_formdata(char *str)
{
	FORMDATA *form = NULL;
	char *ptr = str;

	bool exit_flg = false;
	while(exit_flg != true){
		char *key = NULL;
		char *value = NULL;

		if(!ptr){
			return form;
		}

		if(strstr(ptr,"&")){
			char *s1 = NULL;
			char *s2 = NULL;

			ptr = strsplit(ptr,'&',s1,s2);
			if(strsplit(s1,'=',key,value)){
				form = add_formdata(form,key,value);
			} 
			delete s1;
			delete s2;
		}else{
			if(strsplit(ptr,'=',key,value)){
				form = add_formdata(form,key,value);
				exit_flg = true;
			}
		}

		delete key;
		delete value;
	}

	return form;
}

int main()
{
	FORMDATA *form = parse_formdata("body=main&check=test&ccc=cc=");
	if(!form){
		puts("error");
		return 0;
	}

	// test code
	while(form){
		puts(form->key);
		puts(form->value);
		puts("");

		form = form->next;
	}

	return 0;
}

// created by kimoto.
