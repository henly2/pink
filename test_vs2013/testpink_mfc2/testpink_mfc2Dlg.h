
// testpink_mfc2Dlg.h : 头文件
//

#pragma once

#include <string>

// Ctestpink_mfc2Dlg 对话框
class Ctestpink_mfc2Dlg : public CDialogEx
{
// 构造
public:
	Ctestpink_mfc2Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_TESTPINK_MFC2_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg LRESULT IPCToHost(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT IPCToHost2(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedOk();
    afx_msg void OnDestroy();

    CString m_commandline;
    CString m_filter;
    afx_msg void OnBnClickedButton1();

    std::string m_filter2;
};
