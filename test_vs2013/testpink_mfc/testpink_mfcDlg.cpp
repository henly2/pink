
// testpink_mfcDlg.cpp : ʵ���ļ�
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
    //test_virtual_main();

    // ��Ҫ�������tool����
    //ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);

    // ����͸����
    LONG para = GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE);
    para |= WS_EX_LAYERED;
    SetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE, para);
    SetLayeredWindowAttributes(0, 255, LWA_ALPHA);

    // �����ö�
    //SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	//apihook::StackWalker::Inst().Enable();
    //apihook::gdi_base::EnableHook();
    //apihook::gdi_pen::EnableHook();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void Ctestpink_mfcDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
        apihook::memory_heap::MyStacksIPs_memory::Inst().Dump("memory.leak");

        //apihook::memory_heap::DisableHook();
		//CAboutDlg dlgAbout;
		//dlgAbout.DoModal();
        // ��ӡ��Ҫͨ��DeleteObject�ͷŵ�й©��ַ
        //apihook::gdi_base::MyStacks_base::Inst().Dump("gdi.leak");
        // ��ӡ��Ҫͨ��ReleaseDC�ͷŵ�й©��ַ
        //apihook::gdi_dc::MyStacks_relasedc::Inst().Dump("releasedc.leak");
        // ��ӡ��Ҫͨ��DeleteDC�ͷŵ�й©��ַ
        //apihook::gdi_dc::MyStacks_deletedc::Inst().Dump("deletedc.leak");

        //apihook::gdi_pen::DisableHook();
        //apihook::gdi_base::EnableHook();    
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
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
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
