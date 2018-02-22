
// testpink_mfc2Dlg.cpp : ʵ���ļ�
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

    SetWindowText(REMOTE_WINDOW_TITLE);

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
    apihook::StackWalkerIPC::Inst().EnableRemote(3924);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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