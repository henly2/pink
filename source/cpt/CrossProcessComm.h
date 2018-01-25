/*!
 * \file CrossProcessComm.h
 *
 * \author heng.liu
 * \date 十二月 2017
 *
 * 
 */

#ifndef CrossProcessComm_h__
#define CrossProcessComm_h__

enum CROCSSPROCESSID
{
    CPID_Text = 1,
};

typedef struct _tagCrossProcessData_Text
{
    char  text[100];
    _tagCrossProcessData_Text(){
        memset(this, 0, sizeof(_tagCrossProcessData_Text));
    }
} CrossProcessData_Text;

// 主程序
/* 
1.
afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);

2. 
ON_WM_COPYDATA()

3. 
BOOL CMFCApplication1Dlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
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

        CString text = ((CrossProcessData_Text*)myCDS->lpData)->text;
        oldtext += text;
        m_edit.SetWindowText(oldtext);
        m_edit.SetSel(oldtext.GetLength(), oldtext.GetLength());
    }

    return TRUE;
}
*/

#endif // CrossProcessComm_h__