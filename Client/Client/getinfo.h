#include <wtypes.h>
#include <winnt.h>
#include <minwindef.h>
#include <windef.h>
#pragma once
#ifndef __GETINFO_H__
#define __GETINFO_H__



/*----------��������------------*/
//������ǰ������
LPSTR GetCPUInfo (LPSTR output);
LPSTR GetAdapterInfo (LPSTR macAddress);
int GetHardDriveSerialNumber (PCHAR pszIDBuff, int nBuffLen, int nDriveID);
LPSTR  flipAndCodeBytes (LPCSTR str, int pos, int flip, LPSTR buf);
LPSTR GetAllHDSerial (LPSTR output);
LPSTR Byte2char (BYTE num, LPSTR output);
VOID GenerateSerial (unsigned char *output);	//�����Զ��庯���������к�



#endif // !__GETINFO_H__
