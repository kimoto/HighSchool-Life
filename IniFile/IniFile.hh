#ifndef INIFILE_HH_
#define INIFILE_HH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// data chain
typedef struct data{
	char *key;
	char *value;
	struct data *next;
}DATA;

typedef struct section{
	char *name;
	struct data *data;
	struct section *next;
}SECTION;

class IniFile
{
private:
	char *m_FileName;
	SECTION *m_Section;
	char *m_Buffer;

	bool Open(const char *);
	bool Close();

	bool ReadIniFile();
	bool Parse();
public:
	IniFile(const char *);
	~IniFile();

	bool GetValue(const char *, const char *, char *, size_t);
	bool SetValue(const char *, const char *, const char *);

	bool Save();
	bool SaveAs(const char *);

	const char *GetBuffer();
};

#endif
