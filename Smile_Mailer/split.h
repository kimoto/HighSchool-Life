#ifndef _SPLIT_H_INCLUDED_
#define _SPLIT_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus */

/*
 *	メールアドレスからドメイン名を切り出す
 *	成功したら0を返す
 */
extern int SplitDomainName(char *address,char *dest);

/*
 *	フルパスをディレクトリ名だけにする
 */
extern int SplitDirectoryName(char *src,char *result);

#ifdef __cplusplus
}
#endif	/*__cplusplus */

#endif	/* _SPLIT_H_INCLUDED_ */