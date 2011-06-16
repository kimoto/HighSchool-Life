/*
 *	添付ファイル関連の関数
 *	copyright(c) 2004 kimoto
 */
#ifndef _ATTACH_H_INCLUDED_
#define ATTACH_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus */

/*添付ファイルが存在するか調べる*/
extern BOOL CheckFileExist(char *filepath);
/*添付ファイルを読み込む*/
extern BOOL ReadAttachFile(char *filepath,MAILDATA *mdat);


#ifdef __cplusplus
}
#endif	/* __cplusplus	*/

#endif	/* _ATTACH_H_INCLUDED_ */