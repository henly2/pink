
// testpink_mfc.cpp : 定义应用程序的类行为。
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


// Ctestpink_mfcApp 构造

Ctestpink_mfcApp::Ctestpink_mfcApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中


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


// 唯一的一个 Ctestpink_mfcApp 对象

Ctestpink_mfcApp theApp;


// Ctestpink_mfcApp 初始化

BOOL Ctestpink_mfcApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO:  应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	Ctestpink_mfcDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:  在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO:  在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

