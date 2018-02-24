
// testpink_mfc2Dlg.cpp : ʵ���ļ�
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// Ctestpink_mfc2Dlg �Ի���



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


// Ctestpink_mfc2Dlg ��Ϣ�������

BOOL Ctestpink_mfc2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
    SetWindowTextA(GetSafeHwnd(), hook::CLASS_NAME_HOST);
    inithooker();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void Ctestpink_mfc2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    CDialogEx::OnCancel();
}

void Ctestpink_mfc2Dlg::OnBnClickedOk()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
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

    // TODO:  �ڴ˴������Ϣ����������
    uninithooker();
}


void Ctestpink_mfc2Dlg::OnBnClickedButton1()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    UpdateData(TRUE);

    m_filter.MakeLower();
    m_filter2 = hook::WStringToMBytes(m_filter);
}
