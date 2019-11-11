#include "stdafx.h"
#include <Windows.h>
#include "fileoperation.h"
#include "sm2.h"
#include "sm4.h"

BOOL encryptfile (const char *in_fname, unsigned char *key, const char *out_fname)
{
	FILE *fp1, *fp2;
	int i = 0;
	int length = 0;
	//16个字节为一组，分组加密
	unsigned char input[17];
	unsigned char output[16];
	sm4_context ctx;//sm4上下文

	input[16] = '\0';
	sm4_setkey_enc (&ctx, key);//设置key
	fopen_s (&fp1, in_fname, "rb");
	if (fp1 == NULL)
	{
		//printf ("文件打开失败，错误码:%d\n.", GetLastError ());
		return FALSE;
	}
	fopen_s (&fp2,out_fname, "wb");
	if (fp2 == NULL)
	{
		//printf ("文件打开失败，错误码:%d\n.", GetLastError ());
		return FALSE;
	}

	while (1)
	{
		/*每次读入16个字节，使用SM4对称加密算法进行加密*/
		for (i = 0; i < 16; i++)
		{
			if (feof (fp1)) break;	//如果读到文件尾则跳出循环
			input[i] = fgetc (fp1);
		}
		//加密函数的长度参数可能最后一组不是16，但加密之后的输出还是16
		sm4_crypt_ecb (&ctx, SM4_ENCRYPT, 16, input, output);	//加密，输出到output数组,参数2为定义的宏，标志是加密还是解密
		for (i = 0; i < 16; i++)
		{
			fputc (output[i], fp2);		//将加密结果写入到文件中
			output[i] = '\0';			//清除缓冲区
			input[i] = '\0';
		}
		if (feof (fp1)) break;
	}

	fclose (fp1);
	fclose (fp2);
	return TRUE;
}

BOOL decryptfile (const char *in_fname, unsigned char *key, const char *out_fname)
{
	FILE *fp1, *fp2;
	int i = 0;
	unsigned char input[17];
	unsigned char output[17];
	sm4_context ctx; //sm4上下文

	input[16] = '\0';
	output[16] = '\0';
	sm4_setkey_dec (&ctx, key);//设置解密key
	fopen_s (&fp1, in_fname, "rb");
	if (fp1 == NULL)
	{
		//printf ("文件打开失败，错误码:%d\n.", GetLastError ());
		return FALSE;
	}
	fopen_s (&fp2, out_fname, "wb");
	if (fp2 == NULL)
	{
		//printf ("文件打开失败，错误码:%d\n.", GetLastError ());
		return FALSE;
	}
	while (1)
	{
		/*每次读入16个字节，使用SM4对称加密算法进行解密密*/
		for (i = 0; i < 16; i++)
		{
			if (feof (fp1)) break;	//如果读到文件尾则跳出循环
			input[i] = fgetc (fp1);
		}
		if (feof (fp1)) break;	//跳出while循环
		sm4_crypt_ecb (&ctx, SM4_DECRYPT, 16, input, output);
		for (i = 0; i < 16; i++)			//将解密结果写入到文件中
		{
			if ((output[i] == 0xFF) && (output[i + 1] == 0x00) && (output[i + 2] == 0x00))	//识别解密出多余的部分
			{
				//printf ("success！\n");
				break;
			}
			fputc (output[i], fp2);
			//清除缓冲区
			output[i] = '\0';
			input[i] = '\0';
		}
	}

	fclose (fp1);
	fclose (fp2);
	return TRUE;
}

//嵌入版权信息
BOOL embed_info_in_pdf (const char infilename[20], const char *info, const char outfilename[20])
{
	FILE *infile, *outfile;
	char readline[4096]; //假设一行最大的字节数为4096个
	int iNumObj = 0;	//pdf文件中对象的个数
	char insertpoint[20];
	int len_info = 0;	//计算版权信息的字符串长度，以便修改Obj偏移
	char *newOffsetString = NULL;

	fopen_s (&infile, infilename, "rb");
	fopen_s (&outfile, outfilename, "wb");
	if (infile == NULL || outfile == NULL)
	{
		//printf ("文件无法打开：%s\n", infilename);
		return FALSE;
	}
	iNumObj = getNumofObj (infile);
	_itoa_s (iNumObj - 1, insertpoint, 10); //减1是因为是从对象0开始
	strncat_s (insertpoint, " 0 obj\n", 7);
	len_info = strlen (info);
	//printf ("插入点：%s", insertpoint);
	//pdf文件的格式比较明确，每个对象的开头和结尾都独占一行，所以只要一行一行读文件，匹配到对象头时，先插入版权信息即可
	while (1)
	{
		//TODO:需要找到PDF文件对象的数目，<</Size 17..即表示有17个对象，将版权信息嵌入最后一个对象，16 0 obj
		my_fgets (readline, infile);
		if (strncmp (readline, "%%EOF", 5) == 0)
		{
			//printf ("END\n");
			my_fputs (readline, outfile);
			break;
		}
		//找到插入点，嵌入版权信息，这里嵌入的是明文的版权信息，后续自个儿改进可以将版权信息转换文20,09的十六进制数据，便于隐藏
		else if (strcmp (readline, insertpoint) == 0)
		{
			//printf ("已嵌入版权信息！\n");
			my_fputs ((char *)info, outfile);
			my_fputs (readline, outfile);
		}
		//修改对象偏移量
		else if (strncmp (readline, "trailer", 7) == 0)
		{
			fseek (infile, -28L, SEEK_CUR); //文件指针向前移动28个字符
			fseek (outfile, -20L, SEEK_CUR); //此时输出文件还未写入trailer
			memset (readline, 0, sizeof (readline));
			my_fgets (readline, infile);
			newOffsetString = getOffset (readline, len_info); //从一行数据中提取offset
			my_fputs (newOffsetString, outfile);
			//读取trailer这一行并写入
			memset (readline, 0, sizeof (readline));
			my_fgets (readline, infile);
			my_fputs (readline, outfile);
		}
		else
		{
			my_fputs (readline, outfile);
		}
		//清空字符串以便下一次写入新的行
		memset (readline, 0, sizeof (readline));
	}
	fclose (infile);
	fclose (outfile);
	return TRUE;
}

//提取版权信息
BOOL extract_info_in_pdf (const char *stegofile, char *out_CopyRightinfo)
{
	FILE *fpStegofile;
	char readline[4096];
	int flag = 0;

	fopen_s (&fpStegofile, stegofile, "rb");
	if (fpStegofile == NULL)
	{
		//printf ("无法打开Stego文件\n");
		return FALSE;
	}
	while (1)
	{
		memset (readline, 0, sizeof (readline)); //清除缓存区
		my_fgets (readline, fpStegofile);
		if (strncmp (readline, "Copyright", 9) == 0)
		{
			//使用安全的函数strncpy_s
			strncpy_s (out_CopyRightinfo, 64, readline, 4096);
			flag = 1;
			break;
		}
		if (feof (fpStegofile)) break;
	}
	if (!flag)
	{
		//printf ("未找到版权信息！\n");
		fclose (fpStegofile);
		return FALSE;
	}
	else
	{
		//测试输出
		//printf ("文件版权信息：%s\n", CopyRight_str);
		fclose (fpStegofile);
		return TRUE;
	}
}

void my_fgets (char *Buffer, FILE *fp)
{
	int ch;
	for (int i = 0; i < 4096; i++)
	{
		ch = fgetc (fp);
		if (ch == '\n')
		{
			Buffer[i] = ch;
			break;
		}
		if (ch == EOF)
		{
			break;
		}
		Buffer[i] = ch;
	}
}

void my_fputs (char *Buffer, FILE *fp)
{
	int i = 0;
	//单独判断文件结尾
	if (strcmp (Buffer, "%%EOF") == 0)
	{
		fputs (Buffer, fp);
		return;
	}
	while (Buffer[i] != '\n')
	{
		fputc ((int)Buffer[i], fp);
		i++;
	}
	fputc ('\n', fp);
}

//获取文件中的对象数目
int getNumofObj (FILE *fp)
{
	char readline[4096];
	int iNumObj = 0;
	char numStr[10] = { 0 };
	while (1)
	{
		my_fgets (readline, fp);
		if (strncmp (readline, "<</Size", 7) == 0)
		{
			for (int i = 0; i < 10; i++)
			{
				if (!isalnum (readline[i + 8]))
				{
					iNumObj = atoi ((const char *)numStr);
					//测试输出
					//printf ("pdf文件的对象数为%d个\n", iNumObj);
					rewind (fp);	//将文件指针移到文件头
					return iNumObj;
				}
				else
				{
					numStr[i] = readline[i + 8];
				}
			}
		}
	}
}

//获取最后一个对象的偏移
char *getOffset (char *readline, int value)
{
	char *pLineString = NULL;
	char offsetStr[15] = { 0 };
	int offset = 0;

	pLineString = (char *)malloc (20);
	for (int i = 0; i < 15; i++)
	{
		if (!isalnum (readline[i]))
		{
			offset = atoi (offsetStr);
			offset = offset + value; //加上嵌入的消息长度，一般很小，不会改变offset的位数
			snprintf (pLineString, 20, "%010d 00000 n\n", offset);
			//输出测试
			//printf ("偏移字符串：%s\n偏移量：%d\n", pLineString, offset);
			break;
		}
		else
		{
			offsetStr[i] = readline[i];
		}
	}

	return pLineString;
}

void add_file_struct_info_to_file (const char *ercfile, Pdf_File_info pdf_file_info, const char *rarfile)
{
	//将结构体信息嵌入文件中,rarfile是最后打包成功的文件名
	FILE *fpErcfile;
	FILE *fpRarfile;
	int size = 0;
	int ch;

	fopen_s (&fpErcfile, ercfile, "rb");
	fopen_s (&fpRarfile, rarfile, "wb");
	if (fpErcfile == NULL || fpRarfile == NULL)
	{
		//printf ("[+] WARNING:文件无法打开！\n");
		return;
	}
	//按块嵌入RAR文件头部
	fwrite (&pdf_file_info, sizeof (Pdf_File_info), 1, fpRarfile);
	//将ERC文件追加到RAR文件后面
	ch = fgetc (fpErcfile);
	while (ch != EOF)
	{
		fputc (ch, fpRarfile);
		ch = fgetc (fpErcfile);
	}

	fclose (fpErcfile);
	fclose (fpRarfile);
	return;
}

//将16进制形式的字符串转换为unsigned char数组，例："6e7f"-->"\x6e\x7f"
void ConvertHardwareId (char *hex_string, unsigned char *char_str, int size)
{
	char tmp_str[2];

	//Test output
	//printf ("Test for Hardware ID convert:\n");
	for (int i = 0; i < size; i++)
	{
		tmp_str[0] = hex_string[2 * i];
		tmp_str[1] = hex_string[2 * i + 1];
		char_str[i] = htoi (tmp_str);
		//测试输出
		//printf ("%02X", hardware_id[i]);
	}
}

/* 将大写字母转化成小写字母 */
int my_tolower (int c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return c + 'a' - 'A';
	}
	else
	{
		return c;
	}
}

/* 将十六进制字符串转化成十进制整数 */
int htoi (char *s)
{
	int i;
	int n = 0;
	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
	{
		i = 2;
	}
	else
	{
		i = 0;
	}
	for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'); ++i)
	{
		if (my_tolower (s[i]) > '9')
		{
			n = 16 * n + (10 + my_tolower (s[i]) - 'a');
		}
		else
		{
			n = 16 * n + (my_tolower (s[i]) - '0');
		}
	}
	return n;
}

BOOL vertify_and_extract (char *pkgfilename, unsigned char *HardwareID, char *ercfilename)
{
	FILE *fpPkg;
	FILE *fpErc;
	Pdf_File_info *pdf_file_info;
	char data[sizeof(Pdf_File_info)];
	int ch;

	fopen_s (&fpPkg, pkgfilename, "rb");
	fopen_s (&fpErc, ercfilename, "wb");
	if (fpPkg == NULL || fpErc == NULL)
	{
		MessageBoxA (NULL, "文件无法打开！\n", "错误", MB_OK);
	}
	for (int i = 0; i < 0x40; i++)
	{
		data[i] = fgetc (fpPkg);
	}
	pdf_file_info = (Pdf_File_info *)data;
	if (strncmp ((const char *)&pdf_file_info->user_hardware_id, (const char *)HardwareID, 32) == 0)
	{
		//如果验证成功，则提取ERC文件
		ch = fgetc (fpPkg);
		while (ch != EOF)
		{
			fputc (ch, fpErc);
			ch = fgetc (fpPkg);
		}
		fclose (fpErc);
		fclose (fpPkg);
		return TRUE;
	}
	else
	{
		fclose (fpErc);
		fclose (fpPkg);
		return FALSE;
	}
}

/*将TCHAR转为CHAR，便于fopen函数操作*/
void Tchar2char (const TCHAR *tchar, char *_char)
{
	int iLength;
	//获取字节长度   
	iLength = WideCharToMultiByte (CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	//将tchar值赋给_char    
	WideCharToMultiByte (CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);
}

/*将CHAR转为TCHAR*/
void CharToTchar (const char * _char, TCHAR * tchar)
{
	int iLength;
	//同上一个函数
	iLength = MultiByteToWideChar (CP_ACP, 0, _char, strlen (_char) + 1, NULL, 0);
	MultiByteToWideChar (CP_ACP, 0, _char, strlen (_char) + 1, tchar, iLength);
}
