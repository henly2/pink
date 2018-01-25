#include "stdafx.h"

#include "CrossProcessText.h"

#include <strsafe.h>

#include "CrossProcessComm.h"

// Return the window's user data pointer.
template <typename T>
T GetUserDataPtr(HWND hWnd) {
    return reinterpret_cast<T>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

void SetUserDataPtr(HWND hWnd, void* ptr) {
    SetLastError(ERROR_SUCCESS);
    LONG_PTR result =
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ptr));
}

WNDPROC SetWndProcPtr(HWND hWnd, WNDPROC wndProc) {
    WNDPROC old =
        reinterpret_cast<WNDPROC>(::GetWindowLongPtr(hWnd, GWLP_WNDPROC));
    LONG_PTR result = ::SetWindowLongPtr(hWnd, GWLP_WNDPROC,
        reinterpret_cast<LONG_PTR>(wndProc));
    return old;
}

ChildTextHolder::ChildTextHolder()
    : m_edit(NULL)
    , m_edit_wndproc(NULL)
    , m_mainhwnd(NULL)
{

}

ChildTextHolder::~ChildTextHolder()
{

}

// static
LRESULT CALLBACK ChildTextHolder::EditWndProc(HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam) {

    ChildTextHolder* self = GetUserDataPtr<ChildTextHolder*>(hWnd);
    if (nullptr == self)
        return -1;

    switch (message) {
    case WM_CHAR:
        break;
    case WM_NOTIFY:
        //EN_CHANGE
        break;
    case WM_COMMAND:
        //EN_CHANGE
        break;
    case WM_NCDESTROY:
        // Clear the reference to |self|.
        SetUserDataPtr(hWnd, NULL);
        self->m_edit = NULL;
        break;
    }

    return CallWindowProc(self->m_edit_wndproc, hWnd, message, wParam, lParam);
}

bool ChildTextHolder::CreateEdit(HWND hparent, UINT id)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    m_edit = CreateWindow(L"EDIT", NULL,
        WS_CHILD,
        0, 0, 100, 100, 
        hparent, (HMENU)id, hInstance, 0);

    m_edit_wndproc = SetWndProcPtr(m_edit, EditWndProc);

    SetUserDataPtr(m_edit, this);

    return true;
}

void ChildTextHolder::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    if (::GetFocus() != GetEdit()){
        ::SetFocus(GetEdit());
    }

    DWORD mainPid = 0;
    ::GetWindowThreadProcessId(m_mainhwnd, &mainPid);
    if (mainPid != 0)
    {
        CrossProcessData_Text myRec;
        if (wParam != NULL)
            myRec.text[0] = wParam;

        COPYDATASTRUCT myCDS;
        //
        // Fill the COPYDATA structure
        // 
        myCDS.dwData = CPID_Text;          // function identifier
        myCDS.cbData = sizeof(myRec);  // size of data
        myCDS.lpData = &myRec;           // data structure

        ::AllowSetForegroundWindow(mainPid);

        //
        // Call function, passing data in &MyCDS
        //
        ::SendMessage(m_mainhwnd, WM_COPYDATA, (WPARAM)m_edit, (LPARAM)(LPVOID)&myCDS);
    }
}