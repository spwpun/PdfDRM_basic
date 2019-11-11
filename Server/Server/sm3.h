#ifndef _SM3_H_
#define _SM3_H_

/*
* SM3�㷨�����Ĺ�ϣֵ��С����λ���ֽڣ�
*/
#define SM3_HASH_SIZE 32 

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	/*
	* SM3������
	*/
	typedef struct SM3Context
	{
		unsigned int intermediateHash[SM3_HASH_SIZE / 4];
		unsigned char messageBlock[64];
	} SM3Context;

	/*
	* SM3���㺯��
	*/
	unsigned char *SM3Calc (const unsigned char *message,
		unsigned int messageLen, unsigned char digest[SM3_HASH_SIZE]);

#ifdef __cplusplus
}
#endif //__plusplus

#endif // _SM3_H_
