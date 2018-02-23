// HookerDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include "HwndMessage.h"

SimpleHwndMessage g_hwndmsg;

HMODULE g_hModule = NULL;
HANDLE g_hthread = NULL;
HANDLE g_hexit = NULL;
DWORD WINAPI MessageThread(LPVOID pVoid)
{
    g_hwndmsg.Create(g_hModule, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    g_hwndmsg.Finish();

    if (g_hexit != NULL){
        SetEvent(g_hexit);
    }
    return 0;
}

void MoveMessageWnd(POINT pt);
void StartMessageLoop2(HMODULE hModule)
{
    g_hModule = hModule;
    g_hexit = CreateEventA(NULL, FALSE, FALSE, NULL);
    g_hthread = CreateThread(NULL, 0, MessageThread, NULL, NULL, NULL);
}

void StartMessageLoop(HMODULE hModule, HWND hParent)
{
    // if host process, return;
    HMODULE hprocess = GetModuleHandle(NULL);
    std::string pro_name = hook::GetModuleName(hprocess);
    std::transform(pro_name.begin(), pro_name.end(), pro_name.begin(), tolower);
    if (pro_name == "hooker.exe"){
        return;
    }
    if (pro_name == "testpink_mfc2.exe"){
        return;
    }

    g_hModule = hModule;

    g_hwndmsg.Create(g_hModule, hParent);

    POINT pt = { 0, 0 };
    ClientToScreen(hParent, &pt);
    MoveMessageWnd(pt);
}

void StopMessageLoop(HMODULE hModule)
{
    // if host process, return;
    HMODULE hprocess = GetModuleHandle(NULL);
    std::string pro_name = hook::GetModuleName(hprocess);
    std::transform(pro_name.begin(), pro_name.end(), pro_name.begin(), tolower);
    if (pro_name == "hooker.exe"){
        return;
    }

    g_hwndmsg.Finish();
    /*
    PostMessage(g_hwndmsg.GetHWND(), WM_QUIT, NULL, NULL);

    if (g_hexit != NULL){
        WaitForSingleObject(g_hexit, INFINITE);
        CloseHandle(g_hexit);
        g_hexit = NULL;
    }

    if (g_hthread != NULL){
        CloseHandle(g_hthread);
        g_hthread = NULL;
    }*/
}

void MoveMessageWnd(POINT pt)
{
    HWND hwnd = g_hwndmsg.GetHWND();
    if (hwnd != NULL)
    {
        RECT rt;
        GetClientRect(hwnd, &rt);
        SetWindowPos(hwnd, NULL, pt.x-rt.right, pt.y-rt.bottom, 0, 0 , SWP_NOSIZE|SWP_NOZORDER);
    }
}

HWND GetMessageWnd()
{
    return g_hwndmsg.GetHWND();
}