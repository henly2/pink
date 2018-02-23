#pragma once

#include <Windows.h>

#include <string>

#include "../source/apihook/APIHook.hpp"
#include "../source/apihook/Gdi/Gdi.hpp"

#include "../source/apihook/APIHook2.hpp"
#include "../source/apihook/memory/MemoryHook.hpp"

#include "../Hooker/common.h"

using namespace hook;

std::string g_tips;

class SimpleHwndMessage
{
    HINSTANCE g_hinst;
    HWND g_hwnd;
    unsigned int g_msg;

    DWORD g_pid;
    std::string g_processname;

    static SimpleHwndMessage* s_SimpleHwndMessage;
public:
    SimpleHwndMessage()
        : g_hinst(NULL)
        , g_hwnd(NULL)
        , g_msg(0)
        , g_pid(0)
    {
        s_SimpleHwndMessage = this;
    }
    ~SimpleHwndMessage()
    {
    }

public:
    // 创建消息窗口
    bool Create(HINSTANCE hinstance)
    {
        g_hinst = hinstance;

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
            NULL, NULL, g_hinst, NULL);
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

        if (g_msg == 0)
        {
            g_msg = RegisterWindowMessage("WM_HOOKER_SS");
        }

        return true;
    }

    void SetTarget(DWORD pid, HMODULE hmoudle)
    {
        g_pid = pid;
        if (pid == 0)
        {
            g_processname.clear();
            apihook::StackWalkerIPC::Inst().DisableRemote();
        }
        else
        {
            g_processname = GetModuleName(hmoudle);
            apihook::StackWalkerIPC::Inst().EnableRemote(pid);
        }
        
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

        g_tips = "HandleIPCHost2";

        return 0;
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
    {
        int wmId, wmEvent;
        PAINTSTRUCT ps;
        HDC hdc;

        if (nMsg == s_SimpleHwndMessage->g_msg)
        {
            s_SimpleHwndMessage->HandleIPCHost2(wParam, lParam);
            InvalidateRect(hWnd, NULL, TRUE);

            return 0;
        }

        switch (nMsg)
        {
        case WM_IPC_TOHOST:
            s_SimpleHwndMessage->HandleIPCHost(wParam, lParam);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            // TODO:  在此添加任意绘图代码...
            {
                std::string text = "hooker: ";
                text += g_tips;

                RECT rt;
                GetClientRect(hWnd, &rt);
                DrawTextA(hdc, text.c_str(), text.length(), &rt, 0);
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