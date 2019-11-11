// Server.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "fileoperation.h"
#include "sm4.h"
#include "Server.h"
#include <commdlg.h>

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
TCHAR szPdfFileName[MAX_PATH] = { 0 };			// 要打包的PDF文件
TCHAR szStegFileName[MAX_PATH] = { 0 };			// 嵌入版权信息后的steg文件
TCHAR szErcFileName[MAX_PATH] = { 0 };			// 加密之后的ERC文件
OPENFILENAME ofn_pdf;							// 原始pdf
OPENFILENAME ofn_pdf_with_copyright;			// 嵌入版权信息后的pdf
Pdf_File_info com_pdf_info = { 0 };				// PDF文件信息结构体
Rights_Claimed file_rights = { 0 };				// 文件版权声明
sm4_context sm4_ctx;							// SM4加密算法的上下文

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Launch (HWND, UINT, WPARAM, LPARAM);

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
    LoadStringW(hInstance, IDC_SERVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SERVER));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SERVER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SERVER);
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
//  目标: 处理主窗口的消息。
//
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
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case IDM_LAUNCH:
				DialogBox (hInst, MAKEINTRESOURCE (IDD_DIALOG1), hWnd, Launch);
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
			TextOutA (hdc, 350, 200, (LPCSTR)"Hello, PDF_DRM_SYSTEM_PROVIDER!", 31);
            EndPaint(hWnd, &ps);
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

// “关于”框的消息处理程序。
INT_PTR CALLBACK Launch (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char szpkgfilename[MAX_PATH] = { 0 };
	char szpdfFilename[MAX_PATH] = { 0 };
	char szstegfilename[MAX_PATH] = { 0 };
	bool retvalue = false;
	char encrypt_key[33] = { 0 };
	unsigned char key[17] = { 0 };
	HWND hDatetime;
	char szdisplay[64] = { 0 };
	char user_id[65] = { 0 };

	hDatetime = GetDlgItem (hDlg, IDC_DATETIMEPICKER1);
	UNREFERENCED_PARAMETER (lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		//TODO:设置背景
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD (wParam) == IDOK || LOWORD (wParam) == IDCANCEL)
		{
			EndDialog (hDlg, LOWORD (wParam));
			return (INT_PTR)TRUE;
		}
		else
		{
			switch (LOWORD(wParam))
			{
			case IDC_BUTTON1:
				//选择pdf文件
				ZeroMemory (&ofn_pdf, sizeof (ofn_pdf));
				ofn_pdf.lStructSize = sizeof (ofn_pdf);
				ofn_pdf.hwndOwner = hDlg;
				ofn_pdf.lpstrFile = szPdfFileName;
				ofn_pdf.lpstrFile[0] = '\0';
				ofn_pdf.nMaxFile = sizeof (szPdfFileName);
				ofn_pdf.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
				ofn_pdf.nFilterIndex = 1;
				ofn_pdf.lpstrFileTitle = NULL;
				ofn_pdf.nMaxFileTitle = 0;
				ofn_pdf.lpstrInitialDir = NULL;
				ofn_pdf.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				retvalue = GetOpenFileName (&ofn_pdf);
				if (retvalue)
				{
					// 显示到界面中
					SetDlgItemText (hDlg, IDC_EDIT_PDF, ofn_pdf.lpstrFile);
				}
				else
				{
					MessageBoxA (hDlg, "产生意想不到的错误啦！", "错误", MB_OK);
				}
				break;
			case IDC_BUTTON2:
				//嵌入
				GetDlgItemTextA (hDlg, IDC_EDIT1, (LPSTR)file_rights.Rights_info, 64);
				strncat_s ((char *)file_rights.Rights_info, 64, "\n\0", 1);
				Tchar2char ((const TCHAR *)szPdfFileName, szpdfFilename);
				retvalue = embed_info_in_pdf (szpdfFilename, (const char *)file_rights.Rights_info, "stego.pdf");
				if (retvalue)
				{
					MessageBoxA (hDlg, "嵌入成功！", "提示", MB_OK);
				}
				else
				{
					MessageBoxA (hDlg, "嵌入失败！", "提示", MB_OK);
				}
				break;
			case IDC_BUTTON3:
				//选择Stego文件
				ZeroMemory (&ofn_pdf_with_copyright, sizeof (ofn_pdf_with_copyright));
				ofn_pdf_with_copyright.lStructSize = sizeof (ofn_pdf_with_copyright);
				ofn_pdf_with_copyright.hwndOwner = hDlg;
				ofn_pdf_with_copyright.lpstrFile = szStegFileName;
				ofn_pdf_with_copyright.lpstrFile[0] = '\0';
				ofn_pdf_with_copyright.nMaxFile = sizeof (szStegFileName);
				ofn_pdf_with_copyright.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
				ofn_pdf_with_copyright.nFilterIndex = 1;
				ofn_pdf_with_copyright.lpstrFileTitle = NULL;
				ofn_pdf_with_copyright.nMaxFileTitle = 0;
				ofn_pdf_with_copyright.lpstrInitialDir = NULL;
				ofn_pdf_with_copyright.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				retvalue = GetOpenFileName (&ofn_pdf_with_copyright);
				if (retvalue)
				{
					// 显示到界面中
					SetDlgItemText (hDlg, IDC_EDIT_STEG, ofn_pdf_with_copyright.lpstrFile);
				}
				else
				{
					MessageBoxA (hDlg, "产生意想不到的错误啦！", "错误", MB_OK);
				}
				break;
			case IDC_BUTTON4:
				//加密
				GetDlgItemTextA (hDlg, IDC_EDIT2, encrypt_key, 33);
				ConvertHardwareId (encrypt_key, key, 17);
				Tchar2char (szStegFileName, szstegfilename);
				retvalue = encryptfile (szstegfilename, key, "secret.erc");
				if (retvalue)
				{
					MessageBoxA (hDlg, "加密成功！", "提示", MB_OK);
				}
				else
				{
					MessageBoxA (hDlg, "加密失败！", "提示", MB_OK);
				}
				break;
			case IDC_BUTTON5:
				//设置
				GetDlgItemTextA (hDlg, IDC_EDIT3, (LPSTR)com_pdf_info.user, 10);
				GetDlgItemTextA (hDlg, IDC_EDIT4, (LPSTR)com_pdf_info.creator, 10);
				GetDlgItemTextA (hDlg, IDC_EDIT5, user_id, 65);
				// 转换为16进制的实际数据
				ConvertHardwareId (user_id, com_pdf_info.user_hardware_id, 32); 
				//TODO:获取时间数据
				MessageBoxA (hDlg, "设置成功！", "提示", MB_OK);
				break;
			case IDC_BUTTON6:
				//打包
				GetDlgItemTextA (hDlg, IDC_EDIT6, szpkgfilename, MAX_PATH);
				retvalue = add_file_struct_info_to_file ("secret.erc", com_pdf_info, (const char *)szpkgfilename);
				wsprintfA (szdisplay, "已生成打包文件（.pkg）:%s", szpkgfilename);
				if (retvalue)
				{
					MessageBoxA (hDlg, szdisplay, "提示", MB_OK);
				}
				else
				{
					MessageBoxA (hDlg, "发生致命错误！", "提示", MB_OK);
				}
				break;
			default:
				break;
			}
		}
		break;
	case WM_CTLCOLORDLG:
		//TODO:返回画笔句柄
		break;
	}
	return (INT_PTR)FALSE;
}
