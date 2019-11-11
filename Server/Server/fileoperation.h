#pragma once
#ifndef __FILEOPERATION_HEADER__
#define __FILEOPERATION_HEADER__
#include <wtypes.h>
#include <stdio.h>

/*-------------------struct define------------------*/
//该结构体由服务端构造，添加在pdf文件头部，客户端提取pdf文件的时候将此部分信息略过
struct Pdf_File_info
{
	unsigned char user[10];	//使用者
	unsigned char creator[10];	//创建者
	unsigned char user_hardware_id[32]; //使用者的硬件序列号，可用来验证
	DATE create_time;	//创建日期
};

//该结构体的信息利用信息隐藏算法隐藏到pdf文件中去，作为版权声明
struct Rights_Claimed
{
	unsigned char Rights_info[64];	//声明例：Copyright by spwpun 2019-05-15
};

/*------------------------funtion declaration-----------------*/
BOOL encryptfile (const char *in_fname, unsigned char *key, const char *out_fname);
BOOL decryptfile (const char *in_fname, unsigned char *key, const char *out_fname);
BOOL embed_info_in_pdf (const char infilename[20], const char *info, const char outfilename[20]);
BOOL extract_info_in_pdf (const char *stegofile, char *out_CopyRightinfo);
BOOL add_file_struct_info_to_file (const char *ercfile, Pdf_File_info pdf_file_info, const char *rarfile);

void my_fputs (char *Buffer, FILE *fp);
void my_fgets (char *Buffer, FILE *fp);
int getNumofObj (FILE *fp);
char *getOffset (char *readline, int value);
void ConvertHardwareId (char *hex_string, unsigned char *hardware_id, int size);
int my_tolower (int c);
int htoi (char *s);
BOOL vertify_and_extract (char *pkgfilename, unsigned char *HardwareID, char *ercfilename);
void Tchar2char (const TCHAR *tchar, char *_char);
void CharToTchar (const char * _char, TCHAR * tchar);

#endif // !__FILEOPERATION_HEADER__
