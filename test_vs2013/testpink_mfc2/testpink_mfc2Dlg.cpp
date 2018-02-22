
// testpink_mfc2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "testpink_mfc2.h"
#include "testpink_mfc2Dlg.h"
#include "afxdialogex.h"

#include <fstream>

#include "../source/apihook/APIHook2.hpp"
#include "../source/apihook/memory/MemoryHook.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_IPC2  (WM_USER+10)

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
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Ctestpink_mfc2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Ctestpink_mfc2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_IPC2, OnRecordCallstack)
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

    SetWindowText(REMOTE_WINDOW_TITLE);

	// TODO:  在此添加额外的初始化代码
    apihook::StackWalkerIPC::Inst().EnableRemote(3924);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void Ctestpink_mfc2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();

        apihook::memory_heap::MyStacks_memory::Inst().Dump("memory.leak");
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

LRESULT Ctestpink_mfc2Dlg::OnRecordCallstack(WPARAM wParam, LPARAM lParam)
{
    apihook::StackWalkerIPC::ContextIPC cs;
    if (false == apihook::StackWalkerIPC::Inst().ReadSharedMemory(cs))
        return -1;

    if (cs.type == -1)
    {
        apihook::memory_heap::MyStacks_memory::Inst().Remove(cs.addr);
        return 0;
    }
    else if (cs.type == 1)
    {
        std::string stacks = apihook::StackWalkerIPC::Inst().WalkerRemote(cs);
        
        apihook::memory_heap::MyStacks_memory::Inst().Add("heap", cs.addr, stacks);

        return 0;
    }

    
    
    return 0;
}