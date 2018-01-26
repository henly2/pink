
// testpink_mfcDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "testpink_mfc.h"
#include "testpink_mfcDlg.h"
#include "afxdialogex.h"

#include "../source/cpt/CrossProcessComm.h"
#include "../include/log/console_log.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
// https://www.codeproject.com/tips/76427/how-to-bring-window-to-top-with-setforegroundwindo
/*
��һ�����̲��Ǵ������뽹�������£��Լ���ĳ����������ö�����ȡ���뽹���ǲ�������ģ�
��Ϊ���������������û��Ĳ���������һ�㶼�ǻ�����������˸��֪ͨ�û��Լ�ѡ��
��Ȼ�����ActiveThisProcessToForeground����ǿ����ɡ�

���ǻ�����������һ����������磺2����������صģ���һ�������ڲ���ʱ������ϣ�����ڶ�����ȡ���뽹��ģ����������߼���
1. ����1�����������2��ȡ����
2. ����2SetForegroundWindow
3. ������Ի�ȡ���������������Ļ���2�����һ�����������룬һ����Ӣ�����룬�ͻ����νӲ��õ���������ҽ���1��������ĵĻ������뷨��һֱ���ڡ�����
4. Ҳ������cptĿ¼�µķ������ڽ���1����һ�����ص�edit��������仯ʱ�����ı�copydata������2�������޷����2�����뷨��һ�µ�ȱ��
*/

// ���Ƽ�ʹ��
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
    ~CAboutDlg();

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


// Ctestpink_mfcDlg �Ի���



Ctestpink_mfcDlg::Ctestpink_mfcDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Ctestpink_mfcDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    util::Console_log::ins().init();
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
END_MESSAGE_MAP()


// Ctestpink_mfcDlg ��Ϣ�������

BOOL Ctestpink_mfcDlg::OnInitDialog()
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
    SetWindowText(_T("Ctestpink_mfcDlg"));

    m_edit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(10, 10, 100, 100), this, 1);

    CRect rt;
    GetClientRect(&rt);
    ClientToScreen(&rt);
    HWND hwnd = GetSafeHwnd();
    // ����ȥ������߿�����ھ���ģʽ��������
    /*
    // ����ԭ����λ��
    ::MoveWindow(hwnd, rt.left, rt.top, rt.Width(), rt.Height(), FALSE);
    // ȥ������߿�
    ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE)&(~(WS_BORDER | WS_CAPTION)));
    // ���λ�ú�ԭ��һ��������ģʽ�±��Ż������û����Ч��ֱ���´ε�OnSize�Ż���Ч
    ::MoveWindow(hwnd, rt.left, rt.top, rt.Width(), rt.Height(), TRUE);
    */

    // ����ȥ������߿�����ھ���ģʽ�¿���
    /*
    // ����ԭ����λ��
    ::MoveWindow(hwnd, rt.left, rt.top, rt.Width()-1, rt.Height()-1, FALSE);
    // ȥ������߿�
    ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE)&(~(WS_BORDER | WS_CAPTION)));
    // ���λ�ú�ԭ��һ��������ģʽ�±��Ż������û����Ч��ֱ���´ε�OnSize�Ż���Ч
    ::MoveWindow(hwnd, rt.left, rt.top, rt.Width(), rt.Height(), TRUE);
    */


    //test_bind_main();
    test_virtual_main();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void Ctestpink_mfcDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void Ctestpink_mfcDlg::OnPaint()
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