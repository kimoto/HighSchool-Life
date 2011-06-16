#include "IniFile.hh"

IniFile::IniFile(const char *file)
{
	m_FileName = NULL;
	m_Section = NULL;
	m_Buffer = NULL;

	Open(file);
	ReadIniFile();
	Parse();
	return;
}

IniFile::~IniFile()
{
	Close();
	return;
}

bool
IniFile::Open(const char *file)
{
	if(!file)
		return false;
	//m_FileName = file;
	int len = strlen(file);
	m_FileName = new char[len+1];
	strncpy(m_FileName, file, len);
	m_FileName[len] = '\0';
	return true;
}

bool
IniFile::Close()
{
#define DEL(x) if(x)delete x
	while(m_Section != NULL){
		SECTION *temp = m_Section;
		//printf("section=%s\n", m_Section->name);
		while(m_Section->data != NULL){
			//printf("%s=%s\n",m_Section->data->key,
			//	m_Section->data->value);
			// release
			DEL(m_Section->data->key);
			DEL(m_Section->data->value);

			m_Section->data = m_Section->data->next;
		}
		m_Section = m_Section->next;
		// release
		DEL(temp);
	}

	delete m_FileName;
	delete m_Buffer;
	return true;
}

// beautiful
bool
IniFile::ReadIniFile()
{
	FILE *fp = fopen(m_FileName, "r");
	if(!fp)
		return false;

	for(int size=0;;){
		size += 1024;
		if(size > 50000)
			return false;

		m_Buffer = new char[size + 1];

		int ret = fread(m_Buffer,
			sizeof(char), size, fp);
		if(ret < size){
			m_Buffer[ret] = '\0';
			break;
		}
		delete m_Buffer;
	}
	fclose(fp);
	return true;
}

bool
IniFile::Parse()
{
	SECTION *sect = NULL;

	char *ptr = m_Buffer;
	while(*ptr != '\0'){
		if(*ptr == '\n'){
		}else if(*ptr == '['){
			if(!sect){
				sect = new SECTION;
				sect->next = NULL;
				m_Section = sect;
			}else{
				sect->next = new SECTION;
				sect->next->next = NULL;
				sect = sect->next;
			}
			sect->data = NULL;
		
			// get section name
			int len = strchr(++ptr, ']') - ptr;
			if(len < 0)
				return false;
			sect->name = new char[len + 1];
			if(!sect->name)
				return false;
			strncpy(sect->name, ptr, len);
			sect->name[len] = '\0';
			ptr += len;
		}else{
			// get key and value
			DATA *data;
			if(!sect->data){
				data = new DATA;
				data->next = NULL;
				sect->data = data;
			}else{
				data->next = new DATA;
				data->next->next = NULL;
				data = data->next;
			}
			
			// get key
			int len = strchr(ptr, '=') - ptr;
			if(len < 0)
				return false;
			data->key = new char[len + 1];
			if(!data->key)
				return false;
			strncpy(data->key, ptr, len);
			data->key[len] = '\0';
			ptr += len;

			// get value
			len = strchr(++ptr, '\n') - ptr;
			if(len < 0)
				return false;
			data->value = new char[len + 1];
			if(!data->value)
				return false;
			strncpy(data->value, ptr, len);
			data->value[len] = '\0';
			ptr += len;
		}
		ptr++;
	}
	return true;
}

bool
IniFile::GetValue(const char *section,
			const char *key, char *buffer, size_t buf_size)
{
	SECTION *sect = m_Section;
	while(sect != NULL){
		// section find!!
		if(strncmp(sect->name, section,
			strlen(sect->name)) == 0){
			break;
		}
		sect = sect->next;
	}
	// not found
	if(sect == NULL){
		return false;
	}
	
	DATA *data = sect->data;
	while(data != NULL){
		if(strncmp(data->key, key,
			strlen(data->key)) == 0){
			strncpy(buffer, data->value, buf_size);
			return true;
		}
		data = data->next;
	}
	return false;
}

bool
IniFile::SetValue(const char *section,
			const char *key, const char *value)
{
	SECTION *sect = m_Section;
	bool flag = false;
	while(true){
		if(strncmp(sect->name, section, strlen(sect->name))==0){
			flag = true;
			break;
		}
		if(sect->next == NULL){
			break;
		}
		sect = sect->next;
	}
	//not found
	if(flag == false){
		SECTION *append = new SECTION;
		append->data = new DATA;
		append->name = new char[strlen(section)+1];
		append->data->key = new char[strlen(key)+1];
		append->data->value = new char[strlen(value)+1];

		strncpy(append->name, section, strlen(section));
		strncpy(append->data->key, key, strlen(key));
		strncpy(append->data->value, value, strlen(value));

		append->next = NULL;
		append->data->next = NULL;

		sect->next = append;
		return true;
	}

	flag = false;
	DATA *data = sect->data;
	while(true){
		if(strncmp(data->key, key, strlen(data->key))==0){
			puts("truetrue!!");
			flag = true;
			break;
		}
		if(data->next == NULL){
			break;
		}
		data = data->next;
	}
	if(flag == false){
		DATA *append = new DATA;
		append->key = new char[strlen(key)+1];
		append->value = new char[strlen(value)+1];
		strncpy(append->key, key, strlen(key));
		strncpy(append->value, value, strlen(value));

		data->next = append;
		append->next = NULL;
	}else{
		delete data->value;
		data->value = new char[strlen(value)+1];
		strncpy(data->value, value, strlen(value));
	}
	return true;
}

const char *
IniFile::GetBuffer()
{
	return m_Buffer;
}

bool
IniFile::Save()
{
	SaveAs(m_FileName);
	return true;
}

bool
IniFile::SaveAs(const char *file)
{
	FILE *fp = fopen(file, "w");
	if(!fp)
		return false;

	SECTION *sect = m_Section;
	while(sect != NULL){
		fprintf(fp, "[%s]\n", sect->name);

		DATA *data = sect->data;
		while(data != NULL){
			fprintf(fp, "%s=%s\n",
				data->key, data->value);
			data = data->next;
		}
		fprintf(fp, "\n");
		sect = sect->next;
	}
		
	fclose(fp);
	return true;
}
