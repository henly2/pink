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

void StartMessageLoop(HMODULE hModule)
{
    g_hModule = hModule;
    g_hexit = CreateEventA(NULL, FALSE, FALSE, NULL);
    g_hthread = CreateThread(NULL, 0, MessageThread, NULL, NULL, NULL);
}

void StopMessageLoop(HMODULE hModule)
{
    PostMessage(g_hwndmsg.GetHWND(), WM_QUIT, NULL, NULL);

    if (g_hexit != NULL){
        WaitForSingleObject(g_hexit, INFINITE);
        CloseHandle(g_hexit);
        g_hexit = NULL;
    }

    if (g_hthread != NULL){
        CloseHandle(g_hthread);
        g_hthread = NULL;
    }
}