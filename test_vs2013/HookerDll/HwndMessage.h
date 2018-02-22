#pragma once

#include <Windows.h>

#include <string>

#include "../source/apihook/APIHook.hpp"
#include "../source/apihook/Gdi/Gdi.hpp"

#include "../source/apihook/APIHook2.hpp"
#include "../source/apihook/memory/MemoryHook.hpp"

#include "../Hooker/common.h"

using namespace hook;

HINSTANCE g_hinst = NULL;
HWND g_hwnd = NULL;
std::string g_tips;

DWORD g_process_id = 0;
std::string g_processname;
class SimpleHwndMessage
{
public:
    SimpleHwndMessage()
    {
    }
    ~SimpleHwndMessage()
    {
    }

public:
    // 创建消息窗口
    bool Create(HINSTANCE hinstance, HWND parent = NULL)
    {
        g_process_id = GetCurrentProcessId();
        g_processname = GetModuleName(GetModuleHandleA(NULL));

        g_hinst = hinstance;

        // register class
        WNDCLASSEXA wex = { 0 };
        wex.cbSize = sizeof(WNDCLASSEXA);

        wex.style = CS_HREDRAW | CS_VREDRAW;
       
        wex.cbClsExtra = 0;
        wex.cbWndExtra = 0;
        wex.lpfnWndProc = WndProc;
        wex.hInstance = g_hinst;
        wex.lpszClassName = CLASS_NAME_DLL;
        wex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        if ((RegisterClassExA(&wex)) == NULL)
        {
            return false;
        }
        // create window
        g_hwnd = CreateWindowA(CLASS_NAME_DLL, NULL, WS_POPUP | WS_THICKFRAME,
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
        int t = 0;
        SetWindowPos(g_hwnd, HWND_TOPMOST, l, t, w, h, SWP_SHOWWINDOW);
        UpdateWindow(g_hwnd);

        apihook::StackWalker::Inst().Enable();
        apihook::gdi_base::EnableHook();

        apihook::StackWalkerIPC::Inst().EnableLocal(CLASS_NAME_HOST, WM_IPC_TOHOST2);
        return true;
    }

    void Finish()
    {
        apihook::gdi_bitmap::DisableHook();
        apihook::gdi_brush::DisableHook();
        apihook::gdi_dc::DisableHook();
        apihook::gdi_extpen::DisableHook();
        apihook::gdi_font::DisableHook();
        apihook::gdi_palette::DisableHook();
        apihook::gdi_pen::DisableHook();
        apihook::gdi_region::DisableHook();

        apihook::gdi_base::DisableHook();
        apihook::StackWalker::Inst().Disable();

        apihook::gdi_base::MyStacks_base::Inst().Clear();
        apihook::gdi_dc::MyStacks_relasedc::Inst().Clear();
        apihook::gdi_dc::MyStacks_deletedc::Inst().Clear();

        apihook::memory_heap::DisableHook();
        apihook::StackWalkerIPC::Inst().DisableLocal();
    }

    // 获取窗口
    HWND GetHWND() const
    {
        return g_hwnd;
    }

private:
#define HE_IS_GDIID(value, item) if (IS_GDIID(value, item))   \
        { \
            apihook::gdi_##item::EnableHook();\
            g_tips = "enable ";\
            g_tips += #item;\
            InvalidateRect(g_hwnd, NULL, TRUE);\
        }
    static void he(int value)
    {  
        HE_IS_GDIID(value, bitmap)
        HE_IS_GDIID(value, brush)
        HE_IS_GDIID(value, dc)
        HE_IS_GDIID(value, extpen)
        HE_IS_GDIID(value, font)
        HE_IS_GDIID(value, palette)
        HE_IS_GDIID(value, pen)
        HE_IS_GDIID(value, region)
    }
#define HD_IS_GDIID(value, item) if (IS_GDIID(value, item))   \
        { \
            apihook::gdi_##item::DisableHook();\
            g_tips = "disable ";\
            g_tips += #item;\
            InvalidateRect(g_hwnd, NULL, TRUE);\
        }
    static void hd(int value)
    {
        HD_IS_GDIID(value, bitmap)  
        HD_IS_GDIID(value, brush)    
        HD_IS_GDIID(value, dc)       
        HD_IS_GDIID(value, extpen)   
        HD_IS_GDIID(value, font)    
        HD_IS_GDIID(value, palette) 
        HD_IS_GDIID(value, pen)    
        HD_IS_GDIID(value, region)  
    }
    static void clear()
    {
        g_tips = "clear...";
        apihook::gdi_base::MyStacks_base::Inst().Clear();
        apihook::gdi_dc::MyStacks_relasedc::Inst().Clear();
        apihook::gdi_dc::MyStacks_deletedc::Inst().Clear();

        g_tips = "clear done...";
        InvalidateRect(g_hwnd, NULL, TRUE);
    }
    static void dump()
    {
        g_tips = "dump...";
        InvalidateRect(g_hwnd, NULL, TRUE);

        std::string dlldir = GetModuleDir(g_hinst);
        dlldir += "\\";
        dlldir += g_processname;
        dlldir += "_";

        apihook::gdi_base::MyStacks_base::Inst().Dump(dlldir + "base.leak");
        apihook::gdi_dc::MyStacks_relasedc::Inst().Dump(dlldir + "releasedc.leak");
        apihook::gdi_dc::MyStacks_deletedc::Inst().Dump(dlldir + "deletedc.leak");

        g_tips = "dump done...";
        InvalidateRect(g_hwnd, NULL, TRUE);
    }

    static void me(int value)
    {
        apihook::memory_heap::EnableHook();
    }
    static void md(int value)
    {
        apihook::memory_heap::DisableHook();
    }

    static void HandleIPC(WPARAM wParam, LPARAM lParam)
    {
        FUNCID funcid = (FUNCID)wParam;
        switch (funcid)
        {
        case hook::Func_he:
            he(lParam);
            break;
        case hook::Func_hd:
            hd(lParam);
            break;
        case hook::Func_clear:
            clear();
            break;
        case hook::Func_dump:
            dump();
            break;
        case hook::Func_me:
            me(lParam);
            break;
        case hook::Func_md:
            md(lParam);
            break;
        default:
            break;
        }

        InvalidateRect(g_hwnd, NULL, TRUE);
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
    {
        int wmId, wmEvent;
        PAINTSTRUCT ps;
        HDC hdc;

        switch (nMsg)
        {
        case WM_IPC_TODLL:
            HandleIPC(wParam, lParam);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            // TODO:  在此添加任意绘图代码...
            {
                std::string text = "I am In Process ";
                text += g_processname;
                text += "--";
                text += std::to_string(GetCurrentProcessId());

                RECT rt;
                GetClientRect(hWnd, &rt);
                rt.bottom /= 2;
                DrawTextA(hdc, text.c_str(), text.length(), &rt, 0);

                rt.top = rt.bottom;
                rt.bottom *= 2;
                DrawTextA(hdc, g_tips.c_str(), g_tips.length(), &rt, 0);
            }
            EndPaint(hWnd, &ps);
            break;
        default:
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
        }

        return 0;
    }
};
