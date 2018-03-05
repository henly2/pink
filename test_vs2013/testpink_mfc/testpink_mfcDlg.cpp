
// testpink_mfcDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "testpink_mfc.h"
#include "testpink_mfcDlg.h"
#include "afxdialogex.h"

#include "../source/cpt/CrossProcessComm.h"
#include "../include/log/console_log.hpp"

//#include "../source/apihook/APIHook.hpp"
//#include "../source/apihook/Gdi/Gdi.hpp"

#include "../Hooker/common.h"
#include "../source/apihook/APIHook2.hpp"
#include "../source/apihook/memory/MemoryHook.hpp"
using namespace apihook;
using namespace hook;

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
// https://www.codeproject.com/tips/76427/how-to-bring-window-to-top-with-setforegroundwindo
/*
当一个进程不是处于输入焦点的情况下，自己在某种情况下想置顶并获取输入焦点是不被允许的；
因为这样的情况会打乱用户的操作，所以一般都是会在任务栏闪烁来通知用户自己选择；
当然下面的ActiveThisProcessToForeground可以强制完成。

我们还会遇到另外一种情况，比如：2个进程是相关的，当一个进程在操作时，我是希望将第二个获取输入焦点的，试试以下逻辑：
1. 进程1设置允许进程2获取焦点
2. 进程2SetForegroundWindow
3. 焦点可以获取，但是如果是输入的话，2边如果一边是中文输入，一边是英文输入，就会有衔接不好的情况，而且进程1如果是中文的话，输入法框还一直存在。。。
4. 也可用用cpt目录下的方法，在进程1创建一个隐藏的edit，当输入变化时，将文本copydata到进程2，还是无法解决2边输入法不一致的缺陷
*/

// 不推荐使用
void ActiveThisProcessToForeground(HWND hWnd)
{
    if (!::IsWindow(hWnd)) return;

    //relation time of SetForegroundWindow lock
    DWORD lockTimeOut = 0;
    HWND  hCurrWnd = ::GetForegroundWindow();
    DWORD dwThisTID = ::GetCurrentThreadId(),
        dwCurrTID = ::GetWindowThreadProcessId(hCurrWnd, 0);

    //we need to bypass some limitations from Microsoft :)
    if (dwThisTID != dwCurrTID)
    {
        ::AttachThreadInput(dwThisTID, dwCurrTID, TRUE);

        ::SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &lockTimeOut, 0);
        ::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0, SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);

        ::AllowSetForegroundWindow(ASFW_ANY);
    }

    ::SetForegroundWindow(hWnd);

    if (dwThisTID != dwCurrTID)
    {
        ::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)lockTimeOut, SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
        ::AttachThreadInput(dwThisTID, dwCurrTID, FALSE);
    }
}

//////////////////////////////////////////////////////////////////////////
extern void test_start_thread();
extern void test_stop_thread();

extern void test_bind_main();
extern void test_virtual_main();

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
    ~CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
    test_start_thread();
}
CAboutDlg::~CAboutDlg()
{
    test_stop_thread();
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Ctestpink_mfcDlg 对话框



Ctestpink_mfcDlg::Ctestpink_mfcDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Ctestpink_mfcDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    //util::Console_log::ins().init();
}

void Ctestpink_mfcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Ctestpink_mfcDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_MESSAGE(1217, OnReceiveNotify)
    ON_WM_COPYDATA()
    ON_BN_CLICKED(IDC_BUTTON1, &Ctestpink_mfcDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// Ctestpink_mfcDlg 消息处理程序

BOOL Ctestpink_mfcDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
    SetWindowText(_T("Ctestpink_mfcDlg"));

    m_edit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(10, 10, 100, 100), this, 1);

    CRect rt;
    GetClientRect(&rt);
    ClientToScreen(&rt);
    HWND hwnd = GetSafeHwnd();
    // 下面去除标题边框代码在经典模式下有问题
    /*
    // 假如原来的位置
    ::MoveWindow(hwnd, rt.left, rt.top, rt.Width(), rt.Height(), FALSE);
    // 去掉标题边框
    ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE)&(~(WS_BORDER | WS_CAPTION)));
    // 如果位置和原来一样，经典模式下被优化，造成没有生效，直到下次的OnSize才会生效
    ::MoveWindow(hwnd, rt.left, rt.top, rt.Width(), rt.Height(), TRUE);
    */

    // 下面去除标题边框代码在经典模式下可以
    /*
    // 假如原来的位置
    ::MoveWindow(hwnd, rt.left, rt.top, rt.Width()-1, rt.Height()-1, FALSE);
    // 去掉标题边框
    ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE)&(~(WS_BORDER | WS_CAPTION)));
    // 如果位置和原来一样，经典模式下被优化，造成没有生效，直到下次的OnSize才会生效
    ::MoveWindow(hwnd, rt.left, rt.top, rt.Width(), rt.Height(), TRUE);
    */


    //test_bind_main();
    //test_virtual_main();

    // 不要激活，设置tool窗口
    //ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);

    // 设置透明度
    LONG para = GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE);
    para |= WS_EX_LAYERED;
    SetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE, para);
    SetLayeredWindowAttributes(0, 255, LWA_ALPHA);

    // 设置置顶
    //SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	//apihook::StackWalker::Inst().Enable();
    //apihook::gdi_base::EnableHook();
    //apihook::gdi_pen::EnableHook();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void Ctestpink_mfcDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
        apihook::memory_heap::MyStacksIPs_memory::Inst().Dump("memory.leak");

        //apihook::memory_heap::DisableHook();
		//CAboutDlg dlgAbout;
		//dlgAbout.DoModal();
        // 打印需要通过DeleteObject释放的泄漏地址
        //apihook::gdi_base::MyStacks_base::Inst().Dump("gdi.leak");
        // 打印需要通过ReleaseDC释放的泄漏地址
        //apihook::gdi_dc::MyStacks_relasedc::Inst().Dump("releasedc.leak");
        // 打印需要通过DeleteDC释放的泄漏地址
        //apihook::gdi_dc::MyStacks_deletedc::Inst().Dump("deletedc.leak");

        //apihook::gdi_pen::DisableHook();
        //apihook::gdi_base::EnableHook();    
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Ctestpink_mfcDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR Ctestpink_mfcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT Ctestpink_mfcDlg::OnReceiveNotify(WPARAM wParam, LPARAM lParam)
{
    //ActiveThisProcessToForeground(this->GetSafeHwnd());
    ::SetForegroundWindow(this->GetSafeHwnd());
    m_edit.SetFocus();

    BYTE vk = static_cast<BYTE>(lParam);
    // Simulate a key press
    keybd_event(vk,
        0x45,
        KEYEVENTF_EXTENDEDKEY | 0,
        0);

    // Simulate a key release
    keybd_event(vk,
        0x45,
        KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
        0);

//     char nChar = (char)wParam;
//     CString text;
//     m_edit.GetWindowText(text);
//     text.AppendChar(nChar);
//     m_edit.SetWindowText(text);
//     m_edit.SetSel(text.GetLength(), text.GetLength());
    return 0;
}

BOOL Ctestpink_mfcDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
    COPYDATASTRUCT* myCDS = pCopyDataStruct;
    if (myCDS->dwData == CPID_Text)
    {
        SetForegroundWindow();
        ActivateTopParent();
        SetFocus();
        m_edit.SetFocus();

        CString oldtext;
        m_edit.GetWindowText(oldtext);

        CString text;// = ((CrossProcessData_Text*)myCDS->lpData)->text;
        oldtext += text;
        m_edit.SetWindowText(oldtext);
        m_edit.SetSel(oldtext.GetLength(), oldtext.GetLength());
    }

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
DWORD WINAPI TestThread(LPVOID pParam)
{
    for (int i = 0; i < 20; i++)
    {
        std::string* pp = new std::string;
        Sleep(200);

        if (i < 10)
        {
            delete pp;
        }
    }
    return 0;
}
DWORD WINAPI TestThread2(LPVOID pParam)
{
    for (int i = 0; i < 20; i++)
    {
        void* pp = malloc(20);
    }
    return 0;
}
DWORD WINAPI TestThread3(LPVOID pParam)
{
    apihook::memory_heap::EnableHook();
    void* pp = malloc(20);
    apihook::memory_heap::DisableHook();

    apihook::memory_heap::MyStacksIPs_memory::Inst().Dump("memory.leak");
    return 0;
}
void Ctestpink_mfcDlg::OnBnClickedButton1()
{
    // TODO:  在此添加控件通知处理程序代码
    //apihook::gdi_base::MyStacks_base::Inst().Clear();
    //apihook::gdi_dc::MyStacks_relasedc::Inst().Clear();
    //apihook::memory_heap::EnableHook();
    //apihook::memory_heap::DisableHook();

    CreateThread(NULL, 0, TestThread3, NULL, 0, NULL);
    /*
    HANDLE h[2];
    for (int i = 0; i < 2; i++)
    {
        //if (i < 1)
        //   h[i] = CreateThread(NULL, 0, TestThread, NULL, 0, NULL);
        //else
           h[i] = CreateThread(NULL, 0, TestThread2, NULL, 0, NULL);
    }
    WaitForMultipleObjects(2, h, TRUE, INFINITE);
    for (int i = 0; i < 2; i++)
    {
        //if (i < 1)
        //   h[i] = CreateThread(NULL, 0, TestThread, NULL, 0, NULL);
        //else
        CloseHandle(h[i]);
    }
    int *ppp = new int;

    int *pppp = new int;

    delete ppp;
    */
    //
    

    return;

    //CPaintDC dc(this);
    CDC* pDC = GetDC();

    //for (int i = 0; i < 100; i++)
    {
        CFont font;
        VERIFY(font.CreateFont(
            24,                        // nHeight
            0,                         // nWidth
            0,                         // nEscapement
            0,                         // nOrientation
            FW_NORMAL,                 // nWeight
            FALSE,                     // bItalic
            FALSE,                     // bUnderline
            0,                         // cStrikeOut
            ANSI_CHARSET,              // nCharSet
            OUT_DEFAULT_PRECIS,        // nOutPrecision
            CLIP_DEFAULT_PRECIS,       // nClipPrecision
            DEFAULT_QUALITY,           // nQuality
            DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
            _T("Arial")));                 // lpszFacename 

        // Done with the font.  Delete the font object.
        //font.DeleteObject();

        //HPEN myPen1;
        //myPen1 = ::CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
        font.Detach();

        HFONT font2 = CreateFontW(
            24,                        // nHeight
            0,                         // nWidth
            0,                         // nEscapement
            0,                         // nOrientation
            FW_NORMAL,                 // nWeight
            FALSE,                     // bItalic
            FALSE,                     // bUnderline
            0,                         // cStrikeOut
            ANSI_CHARSET,              // nCharSet
            OUT_DEFAULT_PRECIS,        // nOutPrecision
            CLIP_DEFAULT_PRECIS,       // nClipPrecision
            DEFAULT_QUALITY,           // nQuality
            DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
            _T("Arial"));


        HPEN hpen = ::CreatePen(PS_SOLID, 2, RGB(255, 0, 0));

        CPen cpen;
        cpen.CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    }
    
    ReleaseDC(pDC);
}
