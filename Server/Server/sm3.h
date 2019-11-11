#ifndef _SM3_H_
#define _SM3_H_

/*
* SM3算法产生的哈希值大小（单位：字节）
*/
#define SM3_HASH_SIZE 32 

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	/*
	* SM3上下文
	*/
	typedef struct SM3Context
	{
		unsigned int intermediateHash[SM3_HASH_SIZE / 4];
		unsigned char messageBlock[64];
	} SM3Context;

	/*
	* SM3计算函数
	*/
	unsigned char *SM3Calc (const unsigned char *message,
		unsigned int messageLen, unsigned char digest[SM3_HASH_SIZE]);

#ifdef __cplusplus
}
#endif //__plusplus

#endif // _SM3_H_
