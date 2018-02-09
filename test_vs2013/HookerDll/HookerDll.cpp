// HookerDll.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "HwndMessage.h"

HMODULE g_hModule = NULL;
SimpleHwndMessage s_hwndmsg;

static DWORD WINAPI MessageThread(LPVOID pVoid)
{
    s_hwndmsg.Create(g_hModule, NULL);

    // ����Ϣѭ��: 
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
    {
        std::string dir = hook::GetModuleDir(hModule);
        OutputDebugStringA("--------------\n");
        OutputDebugStringA(dir.c_str());
        OutputDebugStringA("\n--------------\n");

        std::string name = hook::GetModuleDir(GetModuleHandleA(NULL));
        OutputDebugStringA("--------------\n");
        OutputDebugStringA(name.c_str());
        OutputDebugStringA("\n--------------\n");
    }
    g_hModule = hModule;
    CreateThread(NULL, 0, MessageThread, NULL, NULL, NULL);
}

void StopMessage(HMODULE hModule)
{
    PostMessage(s_hwndmsg.GetHWND(), WM_QUIT, NULL, NULL);
}