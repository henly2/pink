
// testpink_mfc2Dlg.h : ͷ�ļ�
//

#pragma once


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
    afx_msg LRESULT OnRecordCallstack(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
