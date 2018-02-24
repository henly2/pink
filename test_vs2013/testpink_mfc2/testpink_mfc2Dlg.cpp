
// testpink_mfc2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "testpink_mfc2.h"
#include "testpink_mfc2Dlg.h"
#include "afxdialogex.h"

#include <iostream>
#include <string>
#include <fstream>

#include "../Hooker/common.h"
using namespace hook;

#include "../source/apihook/APIHook2.hpp"
#include "../source/apihook/memory/MemoryHook.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern int inithooker();
extern void uninithooker();
extern void docommand(const std::string& input);
extern HANDLE gettarget();

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

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
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Ctestpink_mfc2Dlg 对话框



Ctestpink_mfc2Dlg::Ctestpink_mfc2Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Ctestpink_mfc2Dlg::IDD, pParent)
    , m_commandline(_T(""))
    , m_filter(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Ctestpink_mfc2Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT1, m_commandline);
    DDX_Text(pDX, IDC_EDIT2, m_filter);
}

BEGIN_MESSAGE_MAP(Ctestpink_mfc2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_IPC_TOHOST, IPCToHost)
    ON_MESSAGE(WM_IPC_TOHOST2, IPCToHost2)
    ON_BN_CLICKED(IDCANCEL, &Ctestpink_mfc2Dlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDOK, &Ctestpink_mfc2Dlg::OnBnClickedOk)
    ON_WM_KEYUP()
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BUTTON1, &Ctestpink_mfc2Dlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// Ctestpink_mfc2Dlg 消息处理程序

BOOL Ctestpink_mfc2Dlg::OnInitDialog()
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
    SetWindowTextA(GetSafeHwnd(), hook::CLASS_NAME_HOST);
    inithooker();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void Ctestpink_mfc2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Ctestpink_mfc2Dlg::OnPaint()
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
HCURSOR Ctestpink_mfc2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT Ctestpink_mfc2Dlg::IPCToHost(WPARAM wParam, LPARAM lParam)
{
    FUNCID funcid = (FUNCID)wParam;
    switch (funcid)
    {
    case hook::Func_clear:
        //apihook::memory_heap::MyStacksIPs_memory::Inst().Clear();
        break;
    case hook::Func_dump:
    {
        std::string dlldir = GetModuleDir(GetModuleHandle(NULL));
        std::string exename = GetModuleName((HMODULE)gettarget());
        dlldir += "\\";
        dlldir += exename;
        dlldir += "_";

        //apihook::memory_heap::MyStacks_memory::Inst().Dump(dlldir + "memory.leak");
    }
        break;
    default:
        break;
    }

    return 0;
}

LRESULT Ctestpink_mfc2Dlg::IPCToHost2(WPARAM wParam, LPARAM lParam)
{
    apihook::StackWalkerIPC__::ContextIPC cs;
    if (false == apihook::StackWalkerIPC__::Inst().ReadSharedMemory(cs))
        return -1;

    if (cs.type == -1)
    {
        //apihook::memory_heap::MyStacks_memory::Inst().Remove(cs.addr);
        return 0;
    }
    else if (cs.type == 1)
    {
        std::string stacks = apihook::StackWalkerIPC__::Inst().WalkerRemote(cs);
        //apihook::memory_heap::MyStacks_memory::Inst().Add("heap", cs.addr, stacks);

        return 0;
    }
    
    return 0;
}

void Ctestpink_mfc2Dlg::OnBnClickedCancel()
{
    // TODO:  在此添加控件通知处理程序代码
    CDialogEx::OnCancel();
}

void Ctestpink_mfc2Dlg::OnBnClickedOk()
{
    // TODO:  在此添加控件通知处理程序代码
    //CDialogEx::OnOK();

    UpdateData(TRUE);
    if (m_commandline.IsEmpty())
    {
        std::cout << "input command first" << std::endl;
        return;
    }

    std::string cmdline = hook::WStringToMBytes(m_commandline);
    m_commandline.Empty();
    UpdateData(FALSE);

    docommand(cmdline);
}

void Ctestpink_mfc2Dlg::OnDestroy()
{
    CDialogEx::OnDestroy();

    // TODO:  在此处添加消息处理程序代码
    uninithooker();
}


void Ctestpink_mfc2Dlg::OnBnClickedButton1()
{
    // TODO:  在此添加控件通知处理程序代码
    UpdateData(TRUE);

    m_filter.MakeLower();
    m_filter2 = hook::WStringToMBytes(m_filter);
}
