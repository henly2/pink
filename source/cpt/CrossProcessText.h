/*!
 * \file CrossProcessText.h
 *
 * \author heng.liu
 * \date ʮ���� 2017
 *
 * 
 */

#ifndef CrossProcessText_h__
#define CrossProcessText_h__

#include <map>

// �ӽ���ʹ��
class ChildTextHolder
{
public:
    ChildTextHolder();
    virtual ~ChildTextHolder();

public:
    static LRESULT CALLBACK EditWndProc(HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam);

public:
    void SetMainHwnd(HWND hwnd){
        m_mainhwnd = hwnd;
    }

    bool CreateEdit(HWND hparent, UINT id);

    HWND GetEdit()const{
        return m_edit;
    }

    void OnKeyDown(WPARAM wParam, LPARAM lParam);

private:
    HWND m_edit;
    WNDPROC m_edit_wndproc;

    HWND m_mainhwnd;
};

#endif // CrossProcessText_h__