
// testpink_mfc2Dlg.h : ͷ�ļ�
//

#pragma once

#include <string>

// Ctestpink_mfc2Dlg �Ի���
class Ctestpink_mfc2Dlg : public CDialogEx
{
// ����
public:
	Ctestpink_mfc2Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TESTPINK_MFC2_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
