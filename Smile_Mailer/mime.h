/*
 *	MIME�G���R�[�h�֘A�̊֐�
 *	copyright(c) 2004 kimoto
 */
#ifndef _MIME_H_INCLUDED_
#define _MIME_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus */

/*MIME�̌`����BASE64�G���R�[�h����*/
extern size_t encode_to_mime_base64(const char *src,char *dest);
/*BASE64�G���R�[�h����*/
extern size_t encode_to_base64(const unsigned char *src,unsigned char *dest,int size);

#ifdef __cplusplus
}
#endif	/*__cplusplus */


#endif _MIME_H_INCLUDED_	/*_MIME_H_INCLUDED_*/