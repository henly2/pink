// HookerDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include "HwndMessage.h"

HMODULE g_hModule = NULL;
SimpleHwndMessage s_hwndmsg;

static DWORD WINAPI MessageThread(LPVOID pVoid)
{
    s_hwndmsg.Create(g_hModule, NULL);

    // 主消息循环: 
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    s_hwndmsg.Finish();

    return 0;
}

void StartMessage(HMODULE hModule)
{
    g_hModule = hModule;
    CreateThread(NULL, 0, MessageThread, NULL, NULL, NULL);
}

void StopMessage(HMODULE hModule)
{
    PostMessage(s_hwndmsg.GetHWND(), WM_QUIT, NULL, NULL);
}