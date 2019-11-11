// Client.cpp : 定义应用程序的入口点。

#include "stdafx.h"
#include "getinfo.h"
#include "md5.h"
#include "fileoperation.h"
#include "Client.h"
#include <Commdlg.h>

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
OPENFILENAME ofn = { 0 };								// 打开文件信息结构体
TCHAR szPkgFilename[MAX_PATH];					//打包文件名
char szErcFilename[MAX_PATH] = "";				//提取出的ERC文件名
char szPdfFilename[MAX_PATH] = "";				//提取的pdf文件名
unsigned char HardwareID[33];					//本机硬件ID
unsigned char key[17];							//SM4解密密钥


// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    GetID (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    File_actions (HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息，专门处理消息的WndProc函数部分。
//  CALLBACK 关键字说明消息处理函数是个回调函数，即该函数是由Windows系统调用的，
//  不应该由应用程序调用。
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT: //帮助-->关于
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT: //文件-->退出
                DestroyWindow(hWnd);
                break;
			case IDM_GETID: //文件-->获取硬件信息
				//TODO:嵌入功能函数
				DialogBox (hInst, MAKEINTRESOURCE (IDD_GETID), hWnd, GetID);
				break;
			case IDM_FILE_OPEN:
				DialogBox (hInst, MAKEINTRESOURCE (IDD_FILE_DECRYPT), hWnd, File_actions);
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
			//GDI绘图调用
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
			TextOutA (hdc, 350, 200, (LPCSTR)"Hello, PDF_DRM_SYSTEM_USER!", 27);
            EndPaint(hWnd, &ps); //释放DC(Device Content)句柄
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// “获取硬件ID的消息处理程序”。
INT_PTR CALLBACK GetID (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//TODO:考虑将以下变量改为全局变量
	
	char szDisplay[65];								//用于展示的格式字符串
	HWND dis_List;

	
	memset ((void *)szDisplay, 0, 65);
	dis_List = GetDlgItem (hDlg, IDC_GETID); //获取列表框的句柄
	UNREFERENCED_PARAMETER (lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD (wParam) == IDOK)
		{
			//获取硬件ID函数
			memset ((void *)HardwareID, 0, 33);
			GenerateSerial (HardwareID);
			for (int i = 0; i < 32; i++)
			{
				wsprintfA (&szDisplay[2 * i], "%02X", HardwareID[i]);
			}
			MessageBoxA (hDlg, "成功获取该计算机的硬件ID", "提示", MB_OK);
			SendMessageA (dis_List, LB_ADDSTRING, NULL, (LPARAM)"你的计算机硬件ID如下：");
			SendMessageA (dis_List, LB_ADDSTRING, NULL, (LPARAM)szDisplay);
			memset (szDisplay, 0, 65);
			memset ((void *)key, 0, 17);
			md5_calc (HardwareID, 32, key);
			for (int i = 0; i < 16; i++)
			{
				wsprintfA (&szDisplay[2 * i], "%02X", key[i]);
			}
			SendMessageA (dis_List, LB_ADDSTRING, NULL, (LPARAM)"MD5哈希值如下（请将该值发送给服务方作为你的标识）：");
			SendMessageA (dis_List, LB_ADDSTRING, NULL, (LPARAM)szDisplay);
		}
		else if(LOWORD (wParam) == IDCANCEL)
		{
			EndDialog (hDlg, LOWORD (wParam));
			return (INT_PTR)TRUE;
		}
		break;
	default:
		break;
	}
	return (INT_PTR)FALSE;
}

//文件操作的对话框的消息处理程序。
INT_PTR CALLBACK File_actions (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char szpkgfile[MAX_PATH] = "";
	char szdisplay[64];
	int iErrorCode = 0;
	
	HWND hList;
	HWND hListRecord;
	bool retvalue = false;

	hList = GetDlgItem (hDlg, IDC_LIST1);			//得到选择文件的列表框的句柄
	hListRecord = GetDlgItem (hDlg, IDC_LIST2);
	UNREFERENCED_PARAMETER (lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON2:
			//选择文件打开
			ZeroMemory (&ofn, sizeof (ofn));
			ofn.lStructSize = sizeof (ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = szPkgFilename;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof (szPkgFilename);
			ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			retvalue = GetOpenFileName (&ofn);
			if (retvalue)
			{
				SendMessage (hList, LB_ADDSTRING, NULL, (long)ofn.lpstrFile);
				SendMessageA (hListRecord, LB_ADDSTRING, NULL, (long)"已选择打包文件！");
			}
			else
			{
				MessageBoxA (hDlg, "Failed!", "Error", MB_OK);
			}
			break;
		case IDC_BUTTON_EXTRACT:
			//提取ERC文件，首先需要验证文件头部信息中电脑的硬件ID
			memset ((void *)HardwareID, 0, 33);
			GenerateSerial (HardwareID);
			Tchar2char ((const TCHAR *)ofn.lpstrFile, szpkgfile);
			iErrorCode = GetDlgItemTextA (hDlg, IDC_EDIT1, szErcFilename, MAX_PATH);
			//TODO:对iErrorCode进行判断
			if (vertify_and_extract (szpkgfile, HardwareID, szErcFilename))
			{
				MessageBoxA (hDlg, "验证成功！提取加密文件成功！", "验证提示", MB_OK);
				SendMessageA (hListRecord, LB_ADDSTRING, NULL, (long)"验证成功！提取加密文件成功！");
			}
			else
			{
				MessageBoxA (hDlg, "验证失败，请确保在授权设备上使用！", "验证提示", MB_OK);
				SendMessageA (hListRecord, LB_ADDSTRING, NULL, (long)"验证失败！");
			}
			break;
		case IDC_BUTTON3:
			//使用获取硬件ID后MD5hash过后的值作为key解密文件
			GetDlgItemTextA (hDlg, IDC_EDIT2, szPdfFilename, MAX_PATH);
			//TODO:添加空输入判断
			if (decryptfile (szErcFilename, key, szPdfFilename))
			{
				//CharToTchar (tempfile, szPdfFilename);
				MessageBoxA (hDlg, "解密成功！", "解密提示", MB_OK);
				SendMessageA (hListRecord, LB_ADDSTRING, NULL, (long)"解密成功！");
			}
			else
			{
				MessageBoxA (hDlg, "解密失败，密钥不正确或设备未授权！", "解密提示", MB_OK);
				SendMessageA (hListRecord, LB_ADDSTRING, NULL, (long)"解密失败！");
			}
			break;
		case IDC_BUTTON4:
			//校验版权信息
			memset (szdisplay, 0, 64);
			if (extract_info_in_pdf (szPdfFilename, szdisplay))
			{
				SetDlgItemTextA (hDlg, IDC_EDIT3, szdisplay);
			}
			else
			{
				MessageBoxA (hDlg, "未找到版权信息！", "提示", MB_OK);
			}
			break;
		case IDOK:
			//完成按钮事件
			EndDialog (hDlg, LOWORD (wParam));
			return (INT_PTR)TRUE;
			break;
		case IDCANCEL:
			EndDialog (hDlg, LOWORD (wParam));
			return (INT_PTR)TRUE;
		default:
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}