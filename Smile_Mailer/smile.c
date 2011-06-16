/********************************************************************************************
								Smile-Mailer ver1.0.0
�y�T�v�z
Smile-Mailer�́A�v���o�C�_��SMTP�T�[�o�[�𗘗p�����Ƀ��[���𑗐M����c�[���ł��B
���̂悤�ȗ��_������܂��B

�P�D�v���o�C�_��SMTP�T�[�o�[������ł��鎞�ɍ����ȃ��[�����M�����҂ł��܂��B
�Q�D����̃��[���A�h���X������m���Ă���΂����̂ł���y�ȒP�ݒ�B
�R�D����Ƀ��[�����͂������A���M���ʂ������ɂ킩��B

�y�����z
�P�D�����A�{���A���M�Җ��A���ׂē��{��ɑΉ����Ă��܂��B(JIS)
�Q�D�Y�t�t�@�C���̑��M���\�B
�R�DMFC�����g���Ă��Ȃ����߃v���O�����̃T�C�Y���������A�����B
�S�D�X���b�h���g�p���Ă���̂ŁA���M���Ă���Ԃ��X���[�Y�ȍ�Ƃ��\�B
�T�DVisualStyle�ɑΉ����Ă���B
								
																			2004/07/31
																			program by kimoto
*********************************************************************************************/
#include <windows.h>
#include <windowsx.h>
#include "smtp.h"
#include "smile.h"
#include "mx.h"
#include "attach.h"
#include "split.h"
#include "strtable.h"
#include "config.h"
#include "resource.h"


/*
 *	���C���֐�
 */
BOOL WINAPI WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nShowCmd)
{
	DialogBox(hInstance,MAKEINTRESOURCE(IDD_MAIN),
		NULL,(DLGPROC)MainDlgProc);

	return TRUE;
}


/*
 *	���C���̃_�C�A���O�v���V�[�W��
 */
LRESULT CALLBACK MainDlgProc(HWND hDlg,
						 UINT msg,
						 WPARAM wParam,
						 LPARAM lParam)
{
	static HANDLE hThread;
	static DWORD dwID;

	switch(msg){
	case WM_SENDMAIL:
		/*SendMailThread����̃��b�Z�[�W��������lParam�ɗ���*/
		SetDlgItemText(hDlg,IDC_RESULT,(char *)lParam);
		return TRUE;
	case WM_INITDIALOG:
		InitApplication(hDlg);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDATTACH:
			GetAttachFilePath(hDlg);
			return TRUE;
		case IDSEND:
			/*���[�����M�p�̃X���b�h�̍쐬*/
			hThread = CreateThread(NULL,0,SendMailThread,(LPVOID)hDlg,0,&dwID);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg,0);
			return TRUE;
		}
		return TRUE;
	case WM_DESTROY:
		/*�X���b�h��j������*/
		CloseHandle(hThread);
		ExitApplication(hDlg);
		EndDialog(hDlg,0);
		return TRUE;
	case WM_CLOSE:
		SendMessage(hDlg,WM_DESTROY,0,0);
		return TRUE;
	}
	return FALSE;
}


/*
 *	���̃v���O�����̏���������
 *	��ɁA�ݒ�t�@�C������̓ǂݍ��݂Ɣ��f
 */
BOOL InitApplication(HWND hDlg)
{
	HWND hCombo;
	char buffer[256]="";

	/*�ݒ�t�@�C���̕ۑ��ꏊ�̐ݒ�*/
	InitIniFilePath(ini_filepath);

	/*��{�I�Ȑݒ�̓ǂݍ��݁A�ݒ�*/
	LoadConfig(GetDlgItem(hDlg,IDC_MAILFROM),"MAIL","MAIL FROM",ini_filepath);
	LoadConfig(GetDlgItem(hDlg,IDC_MAILTO),"MAIL","MAIL TO",ini_filepath);
	LoadConfig(GetDlgItem(hDlg,IDC_SENDER),"MAIL","FROM",ini_filepath);
	LoadConfig(GetDlgItem(hDlg,IDC_DNSSERVER),"OPTION","DNS SERVER",ini_filepath);
	LoadConfig(GetDlgItem(hDlg,IDC_PCNAME),"OPTION","PC NAME",ini_filepath);

	/*���[���̗D��x�̏�����*/
	hCombo = GetDlgItem(hDlg,IDC_MAILPRIORITY);
	ComboBox_AddString(hCombo,"1 (Highest)");
	ComboBox_AddString(hCombo,"2 (High)");
	ComboBox_AddString(hCombo,"3 (Normal)");
	ComboBox_AddString(hCombo,"4 (Low)");
	ComboBox_AddString(hCombo,"5 (Lowest)");
	
	/*�D��x��3,Normal�������őI������*/
	ComboBox_SetCurSel(hCombo,2);

	/*���ʂ�\������X�^�e�B�b�N�e�L�X�g�̏�����*/
	StringTableSetWindowText(GetDlgItem(hDlg,IDC_RESULT),IDS_READY);

	return TRUE;
}


/*
 *	ini�t�@�C���̕ۑ��ꏊ�̐ݒ�
 *	�v���O�����̃J�����g�f�B���N�g��
 */
BOOL InitIniFilePath(char *inipath)
{
	char filepath[MAX_PATH]="";
	char directory[MAX_PATH]="";

	GetModuleFileName(NULL,filepath,sizeof(filepath));
	SplitDirectoryName(filepath,directory);
	//MessageBox(NULL,directory,"",MB_OK);

	wsprintf(inipath,"%s\\%s",directory,INI_FILE_NAME);
	//MessageBox(NULL,inipath,"",MB_OK);
	return TRUE;
}

/*
 *	���̃v���O�����̏I������
 */
BOOL ExitApplication(HWND hDlg)
{
	char filepath[1024]="./config.ini";

	SaveConfig(GetDlgItem(hDlg,IDC_MAILFROM),"MAIL","MAIL FROM",filepath);
	SaveConfig(GetDlgItem(hDlg,IDC_MAILTO),"MAIL","MAIL TO",filepath);
	SaveConfig(GetDlgItem(hDlg,IDC_SENDER),"MAIL","FROM",filepath);
	SaveConfig(GetDlgItem(hDlg,IDC_DNSSERVER),"OPTION","DNS SERVER",filepath);
	SaveConfig(GetDlgItem(hDlg,IDC_PCNAME),"OPTION","PC NAME",filepath);	/*2004/07/22:add*/

	return TRUE;
}


/*
 *	���[�����M�X���b�h
 */
DWORD WINAPI SendMailThread(LPVOID lpParam)
{
	MAILDATA mdat;
	HWND hDlg;
	char attach_filepath[MAX_PATH];
	BOOL attach_exist;
	int mailbody_len;			/*�{���̒���*/
	int subject_len;			/*�����̒���*/

	hDlg = (HWND)lpParam;

	if(CheckInputData(hDlg) != TRUE){
		return 0;
	}

	InitMailData(&mdat);

	GetDlgItemText(hDlg,IDC_SENDER,mdat.sender,sizeof(mdat.sender));
	GetDlgItemText(hDlg,IDC_MAILFROM,mdat.from,sizeof(mdat.from));
	GetDlgItemText(hDlg,IDC_MAILTO,mdat.to,sizeof(mdat.to));
	GetDlgItemText(hDlg,IDC_MAILPRIORITY,mdat.priority,sizeof(mdat.priority));
	GetDlgItemText(hDlg,IDC_PCNAME,mdat.pc_name,sizeof(mdat.pc_name));
	strcpy(mdat.mailer,"SMILE MAILER");
	strcpy(mdat.reply,mdat.from);

	/*�Y�t�t�@�C����ǂݍ��ށA*/
	GetDlgItemText(hDlg,IDC_ATTACHFILENAME,attach_filepath,sizeof(attach_filepath));
	attach_exist = CheckFileExist(attach_filepath);
	if(attach_exist == TRUE){
		/*�֐������Ń����������蓖�Ă�̂ŁA�K����ŉ�����邱�ƁB*/
		if(ReadAttachFile(attach_filepath,&mdat) == FALSE){
			StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
					IDS_ERROR_READ_ATTACH_FILE,MB_ICONEXCLAMATION);
		}
	}

	/*�{�����_�C�A���O�{�b�N�X����ǂݍ��ށA�����Y�ꂸ��*/
	mailbody_len = GetWindowTextLength(GetDlgItem(hDlg,IDC_MAILBODY));
	mdat.body = (char *)GlobalAlloc(GMEM_FIXED,mailbody_len + 1);
	GetDlgItemText(hDlg,IDC_MAILBODY,mdat.body,mailbody_len + 1);

	/*�������_�C�A���O�{�b�N�X����ǂݍ��ށA�����Y�ꂸ��*/
	subject_len = GetWindowTextLength(GetDlgItem(hDlg,IDC_MAILSUBJECT));
	mdat.subject = (char *)GlobalAlloc(GMEM_FIXED,subject_len + 1);
	GetDlgItemText(hDlg,IDC_MAILSUBJECT,mdat.subject,subject_len + 1);

	GetMXAndSendMail(hDlg,&mdat,attach_exist);

	GlobalFree(mdat.attach);
	GlobalFree(mdat.body);
	GlobalFree(mdat.subject);

	return 0;
}

/*
 *	MX���R�[�h���擾���āA���̃��[���T�[�o�[�ɐڑ�����
 *	���[�����M������
 */
BOOL GetMXAndSendMail(HWND hDlg,MAILDATA *mdat,BOOL attach_exist)
{
	MXRECORD mx[20];
	LPMXRECORD lpmx;
	char domain[256]="";
	char dns_addr[256]="";

	lpmx = mx;

	/*���M��A�h���X����MX���R�[�h���擾*/
	GetDlgItemText(hDlg,IDC_DNSSERVER,dns_addr,sizeof(dns_addr));
	SplitDomainName(mdat->to,domain);
	StringTableSendMessage(hDlg,WM_SENDMAIL,IDS_QUERY_DNS_SERVER);
	Sleep(300);		/*�������悭�����邽�߁A�������Ă��\��Ȃ��B*/
	
	if(GetMXRecord(domain,dns_addr,lpmx) != GETMX_ERROR_SUCCESS){
		StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
			IDS_ERROR_DNS_SERVER,MB_ICONEXCLAMATION);
		return FALSE;
	}

	/*MX���R�[�h�𗊂��1�����ԂɃ��[�����M�A���������烋�[�v�𔲂���*/
	while(lpmx->flag != FALSE){
		StringTableSendMessage(hDlg,WM_SENDMAIL,IDS_SENDING_MAIL);

		if(SendMail(lpmx->domain,mdat,attach_exist,"===SMILE===")
							== SENDMAIL_ERROR_SUCCESS){
			StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
				IDS_SUCCESS_SENDMAIL,MB_OK);
			return TRUE;
		}
		lpmx++;
	}
	/*�����ɏ���������Ƃ������Ƃ́A���s������*/
	StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
		IDS_ERROR_SENDMAIL,MB_ICONEXCLAMATION);

	return FALSE;
}

/*
 *	�_�C�A���O�{�b�N�X�ɓ��͂��ꂽ�f�[�^�����������̂����ׂ�֐�
 */
BOOL CheckInputData(HWND hDlg)
{
	char buffer[MAX_BUFFER_SIZE]="";

	/*���[�����M��*/
	//GetDlgItemText(hDlg,IDC_MAILFROM,buffer,sizeof(buffer));
	/*if(IsEmpty(buffer)){
		StringTableSendMessage(hDlg,WM_SENDMAIL,IDS_ERROR_MAILFROM_EMPTY);
		return FALSE;
	}*/
	/*���[�����M��*/
	GetDlgItemText(hDlg,IDC_MAILTO,buffer,sizeof(buffer));
	if(IsEmpty(buffer)){
		StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
			IDS_ERROR_MAILTO_EMPTY,MB_ICONEXCLAMATION);
		return FALSE;
	}
	if(IsFoulMailAddr(buffer)){
		StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
			IDS_ERROR_MAILTO_FOUL,MB_ICONEXCLAMATION);
		return FALSE;
	}
	/*DNS�T�[�o�[��IP�A�h���X*/
	GetDlgItemText(hDlg,IDC_DNSSERVER,buffer,sizeof(buffer));
	if(IsEmpty(buffer)){
		StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
			IDS_ERROR_DNS_SERVER_EMPTY,MB_ICONEXCLAMATION);
		return FALSE;
	}
	if(IsFoulIPAddr(buffer)){
		StringTableSendMessageBeep(hDlg,WM_SENDMAIL,
			IDS_ERROR_DNS_SERVER_FOUL,MB_ICONEXCLAMATION);
		return FALSE;
	}
	return TRUE;
}

/*
 *	�w�肵���o�b�t�@���A�󂾂����Ƃ�TRUE
 */
BOOL IsEmpty(char *buffer)
{
	if(strcmp(buffer,"") != 0){
		return FALSE;
	}
	return TRUE;
}

/*
 *	�w�肵���A�h���X���s���Ȍ`����������TRUE
 */
BOOL IsFoulMailAddr(char *address)
{
	char *strptr;

	/*@�̂��Ƃ�.���P�܂܂�Ă��邩���ׂ邾���̒P���ȃA���S���Y��*/
	/*�o�O�͂Ȃ��Ǝv��*/
	strptr = strstr(address,"@");
	if(strptr == NULL){
		return TRUE;
	}
	strptr++;

	strptr = strstr(strptr,".");
	if(strptr == NULL){
		return TRUE;
	}

	return FALSE;
}

/*
 *	�w�肵���A�h���X���AIP�A�h���X(IPv4)�`�������ׂāA
 *	�s���Ȍ`����������TRUE��Ԃ�
 *	(192.168.0.1) => ����������FALSE
 */
BOOL IsFoulIPAddr(char *ip_addr)
{
	char *strptr;

	/*.���R�ȏ゠�邩�ǂ������ׂ邾���̒P���ȃA���S���Y��*/
	strptr = strstr(ip_addr,".");
	if(strptr == NULL){
		return TRUE;
	}
	strptr++;
	
	strptr = strstr(strptr,".");
	if(strptr == NULL){
		return TRUE;
	}
	strptr++;
	
	strptr = strstr(strptr,".");
	if(strptr == NULL){
		return TRUE;
	}
	return FALSE;
}