#pragma once

#include <Windows.h>

#include <string>

#include "../source/apihook/APIHook.hpp"
#include "../source/apihook/Gdi/Gdi.hpp"

#include "../source/apihook/APIHook2.hpp"
#include "../source/apihook/memory/MemoryHook.hpp"

#include "../Hooker/common.h"

using namespace hook;

class SimpleHwndMessage
{
    HINSTANCE g_hinst;
    HWND g_hwnd;

    DWORD g_pid;
    std::string g_processname;

    static SimpleHwndMessage* s_SimpleHwndMessage;
public:
    SimpleHwndMessage()
        : g_hinst(NULL)
        , g_hwnd(NULL)
        , g_pid(0)
    {
        s_SimpleHwndMessage = this;
    }
    ~SimpleHwndMessage()
    {
    }

public:
    // 创建消息窗口
    bool Create(HINSTANCE hinstance, DWORD pid, HMODULE hmoudle, HWND parent = NULL)
    {
        g_hinst = hinstance;

        g_pid = pid;
        g_processname = GetModuleName(hmoudle);

        // register class
        WNDCLASSEXA wex = { 0 };
        wex.cbSize = sizeof(WNDCLASSEXA);

        wex.style = CS_HREDRAW | CS_VREDRAW;
       
        wex.cbClsExtra = 0;
        wex.cbWndExtra = 0;
        wex.lpfnWndProc = WndProc;
        wex.hInstance = g_hinst;
        wex.lpszClassName = CLASS_NAME_HOST;
        wex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        if ((RegisterClassExA(&wex)) == NULL && GetLastError() != 1410)
        {
            return false;
        }
        // create window
        g_hwnd = CreateWindowA(CLASS_NAME_HOST, NULL, WS_POPUP | WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            parent, NULL, g_hinst, NULL);
        if (g_hwnd == NULL)
        {
            return false;
        }

        int w = 300, h = 50;
        HWND hd = GetDesktopWindow();
        RECT rt;
        GetClientRect(hd, &rt);
        int l = rt.right / 2 - w / 2;
        int t = h;
        SetWindowPos(g_hwnd, HWND_TOPMOST, l, t, w, h, SWP_SHOWWINDOW);
        UpdateWindow(g_hwnd);

        apihook::StackWalkerIPC::Inst().EnableRemote(pid);
        return true;
    }

    void Finish()
    {
        apihook::memory_heap::MyStacks_memory::Inst().Clear();

        apihook::StackWalkerIPC::Inst().DisableRemote();
    }

    // 获取窗口
    HWND GetHWND() const
    {
        return g_hwnd;
    }

    void clear()
    {
        apihook::memory_heap::MyStacks_memory::Inst().Clear();
    }
    void dump()
    {
        std::string dlldir = GetModuleDir(g_hinst);
        dlldir += "\\";
        dlldir += g_processname;
        dlldir += "_";

        apihook::memory_heap::MyStacks_memory::Inst().Dump(dlldir + "memory.leak");
    }
    LRESULT HandleIPCHost(WPARAM wParam, LPARAM lParam)
    {
        FUNCID funcid = (FUNCID)wParam;
        switch (funcid)
        {
        case hook::Func_clear:
            clear();
            break;
        case hook::Func_dump:
            dump();
            break;
        default:
            break;
        }

        return 0;
    }
    LRESULT HandleIPCHost2(WPARAM wParam, LPARAM lParam)
    {
        apihook::StackWalkerIPC::ContextIPC cs;
        if (false == apihook::StackWalkerIPC::Inst().ReadSharedMemory(cs))
            return -1;

        if (cs.type == -1)
        {
            apihook::memory_heap::MyStacks_memory::Inst().Remove(cs.addr);
            return 0;
        }
        else if (cs.type == 1)
        {
            std::string stacks = apihook::StackWalkerIPC::Inst().WalkerRemote(cs);

            apihook::memory_heap::MyStacks_memory::Inst().Add("heap", cs.addr, stacks);

            return 0;
        }

        return 0;
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
    {
        int wmId, wmEvent;
        PAINTSTRUCT ps;
        HDC hdc;

        switch (nMsg)
        {
        case WM_IPC_TOHOST:
            s_SimpleHwndMessage->HandleIPCHost(wParam, lParam);
            break;
        case WM_IPC_TOHOST2:
            s_SimpleHwndMessage->HandleIPCHost2(wParam, lParam);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            // TODO:  在此添加任意绘图代码...
            {

            }
            EndPaint(hWnd, &ps);
            break;
        default:
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
        }

        return 0;
    }
};
SimpleHwndMessage* SimpleHwndMessage::s_SimpleHwndMessage = NULL;

//////////////////////////////////////////////////////////////////////////
static SimpleHwndMessage* g_hwndmsg = NULL;

static HINSTANCE g_hinstance = NULL;
static DWORD g_pid = NULL;
static HMODULE g_hmoudle = NULL;
static HWND g_parent = NULL;


static HANDLE g_hthread = NULL;
static HANDLE g_hexit = NULL;
static DWORD WINAPI MessageThread(LPVOID pVoid)
{
    g_hwndmsg->Create(g_hinstance, g_pid, g_hmoudle, g_parent);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    g_hwndmsg->Finish();

    if (g_hexit != NULL){
        SetEvent(g_hexit);
    }
    return 0;
}

static void StartMessageLoop(HINSTANCE hinstance, DWORD pid, HMODULE hmoudle, HWND parent = NULL)
{
    g_hinstance = hinstance;
    g_pid = pid;
    g_hmoudle = hmoudle;
    g_parent = parent;

    g_hwndmsg = new SimpleHwndMessage;

    g_hexit = CreateEventA(NULL, FALSE, FALSE, NULL);
    g_hthread = CreateThread(NULL, 0, MessageThread, NULL, NULL, NULL);
}

static void StopMessageLoop()
{
    if (g_hwndmsg != NULL)
        PostMessage(g_hwndmsg->GetHWND(), WM_QUIT, NULL, NULL);

    if (g_hexit != NULL){
        WaitForSingleObject(g_hexit, INFINITE);
        CloseHandle(g_hexit);
        g_hexit = NULL;
    }

    if (g_hthread != NULL){
        CloseHandle(g_hthread);
        g_hthread = NULL;
    }

    if (g_hwndmsg != NULL)
    {
        delete g_hwndmsg;
        g_hwndmsg = NULL;
    }
}