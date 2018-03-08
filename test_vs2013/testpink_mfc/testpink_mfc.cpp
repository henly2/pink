
// testpink_mfc.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "testpink_mfc.h"
#include "testpink_mfcDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
// Json serialize to struct
// {"a":1, "b":1.2, "c":"cc"}
#include <string>
#include <tuple>

template <typename T>
void FillValue(T value, T value2){
    *value = *value2;
}

template<typename T1, typename T2>
struct SBase{
    SBase(T1* _t1, T2* _t2){
        t1 = _t1;
        t2 = _t2;
    }

    std::tuple<T1*, T2*> tuple_;

    T1* t1;
    T2* t2;

    void FromJson(const std::tuple<T1*, T2*>& tup){
        FillValue(t1, std::get<0>(tup));
        FillValue(t2, std::get<1>(tup));
    }

    void ToJson(std::tuple<T1*, T2*>& tup){
        FillValue(std::get<0>(tup), t1);
        FillValue(std::get<1>(tup), t2);
    }
};

struct STest1
{
    int a;
    double b;
    std::string c;

    STest1()
        : ss(&a, &b)
    {
    }

    SBase<int, double> ss;
};


// Ctestpink_mfcApp

BEGIN_MESSAGE_MAP(Ctestpink_mfcApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// Ctestpink_mfcApp ����

Ctestpink_mfcApp::Ctestpink_mfcApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO:  �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��


    STest1 aa;

    int a = 100;
    double b = 1.2;
    std::tuple<int*, double*> tup(&a, &b);

    aa.ss.FromJson(tup);

    STest1 bb;
    bb.a = -1;
    bb.b = 10.22;
    bb.ss.ToJson(tup);

    OutputDebugStringA("aaa");
}


// Ψһ��һ�� Ctestpink_mfcApp ����

Ctestpink_mfcApp theApp;


// Ctestpink_mfcApp ��ʼ��

BOOL Ctestpink_mfcApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO:  Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	Ctestpink_mfcDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

