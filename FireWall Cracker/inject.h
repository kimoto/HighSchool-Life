/*
 *	DLL�̃C���W�F�N�V�������ȒP�ɍs�����߂̃��C�u����
 *	created by kimoto
 */
#ifndef _INJECT_H_INCLUDED_
#define _INJECT_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif
	
/*
 *	�w�肵���v���Z�X��DLL�𒍓�����
 */
extern BOOL InjectToProcess(TCHAR *cDllName,TCHAR *cTarget);

#ifdef __cplusplus
}
#endif

#endif /*_INJECT_H_INCLUDED_*/