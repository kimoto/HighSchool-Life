#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LENGTH 50000 
#define MAX_BUFFER 1024

typedef struct data{
	char *Key;
	char *Value;
	struct data *Next;
}DATA;

typedef struct section{
	char *Name;
	struct data *Data;
	struct section *Next;
}SECTION;

class CIniFile
{
private:
	char *m_FileName;
	char *m_Buffer;
	int m_MaxLength;
	SECTION *m_Section;

	void Initialize();
	void Terminate();

	int GetSectionNum();
	int GetColumnNum(char *);

	bool GetData();
	bool Parse();
public:
	CIniFile(char *);
	~CIniFile();

	char *ReadString(char *,char *);
	int ReadInt(char *,char *);
};

CIniFile::CIniFile(char *name)
{
	Initialize();

	m_FileName = name;
	if(!GetData()){
		return;
	}
	if(!Parse()){
		return;
	}
	return;
}

CIniFile::~CIniFile()
{
	Terminate();
	return;
}

void
CIniFile::Initialize()
{
	m_FileName = NULL;
	m_Buffer = NULL;
	m_MaxLength = MAX_LENGTH;
	m_Section = NULL;
	return;
}

void
CIniFile::Terminate()
{
	// Release Meory
	if(m_Buffer){
		delete [] m_Buffer;
		m_Buffer = NULL;
	}
	for(SECTION *ptr=m_Section; ptr!=NULL; ){
		SECTION *temp = ptr->Next;
		if(ptr->Name){
			delete [] ptr->Name;
		}
		if(ptr->Data){
			for(DATA *pd=ptr->Data; pd!=NULL; ){
				DATA *temp = pd->Next;
				if(pd->Key){
					delete [] pd->Key;
				}
				if(pd->Value){
					delete [] pd->Value;
				}
				pd = temp;
			}
			delete [] ptr->Data;
		}
		ptr = temp;
	}
	if(m_Section){
		delete [] m_Section;
		m_Section = NULL;
	}
	return;
}

bool
CIniFile::GetData()
{
	FILE *fp;
	fp = fopen(m_FileName,"r");
	if(fp == NULL){
		return false;
	}
	
	for(int size=0;;){
		size += 1024;
		if(m_MaxLength < size){
			return false;
		}
		
		m_Buffer = new char[size];

		int ret = fread(m_Buffer,sizeof(char),size,fp);
		if(ret < size){
			m_Buffer[ret] = 0x00;
			break;
		}

		delete m_Buffer;
	}

	fclose(fp);
	return true;
}

bool
CIniFile::Parse()
{
	// check section number
	m_Section = new SECTION[ GetSectionNum() ];
	SECTION *sect = m_Section;

	for(char *ptr=m_Buffer ; *ptr!=0x00 ; ptr++ ){
		if(*(ptr-1) == '['){
			int len = strchr(ptr,']') - ptr;
			if(len < 0) return false;
			
			sect->Name = new char[len + 1];
			strncpy(sect->Name,ptr,len);
			sect->Name[len + 1] = 0x00;

			// get column line
			ptr = ptr + len + 2;
			int column = GetColumnNum(ptr);
			sect->Data = new DATA[ column ];

			DATA *data = sect->Data;
			for(; column > 0; column--){
				int total = strchr(ptr,'\n') - ptr;
				if(total < 0) return false;

				// '=' forward & back
				int len = strchr(ptr,'=') - ptr;
				data->Key = new char[len];
				strncpy(data->Key,ptr,len);
				data->Key[len] = 0x00;

				len = total - len - 1;
				data->Value = new char[len];
				strncpy(data->Value,ptr+(total-len),len);
				data->Value[len] = 0x00;

				ptr = ptr + total + 1;

				DATA *temp = data;
				data++;
				temp->Next = data;
			}
			data--;
			data->Next = NULL;
			
			SECTION *temp = sect;
			sect++;
			temp->Next = sect;
		}
	}
	sect--;
	sect->Next = NULL;
	
	return true;
}

int
CIniFile::GetSectionNum()
{
	int num = 0;
	for(char *ptr=m_Buffer; *ptr!='\0'; ptr++){
		if(*ptr == '['){
			num++;
			for(; *ptr!=']'; ptr++);
		}
	}
	return num;
}

int
CIniFile::GetColumnNum(char *ptr)
{
	int num = 0;
	while(*ptr != 0x00){
		if(*ptr == '\n'){
			num++;
			if(*(ptr+1) == '\n'){
				break;
			}
		}
		ptr++;
	}
	return num;
}

char *
CIniFile::ReadString(char *section,char *key)
{
	for(SECTION *ptr=m_Section; ptr!=NULL; ptr=ptr->Next){
		if(strcmp(section,ptr->Name)){
			continue;
		}
		for(DATA *pd=ptr->Data; pd!=NULL; pd=pd->Next){
			if(!strcmp(key,pd->Key)){
				return pd->Value;
			}
		}
	}
	return NULL;
}

int
CIniFile::ReadInt(char *section,char *key)
{
	char *res = ReadString(section,key);
	if(!res){
		return -1;
	}
	return atoi(res);
}

// Main Function
int main()
{
	CIniFile *ini = new CIniFile("test.ini");

	puts(ini->ReadString("default","mail"));
	printf("%d\n",ini->ReadInt("default","int"));

	return 0;
}
