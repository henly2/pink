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
        g_hinst = hinstance;

        // register class
        WNDCLASSA wndcls;
        wndcls.cbClsExtra = 0;
        wndcls.cbWndExtra = 0;
        wndcls.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wndcls.hCursor = LoadCursor(NULL, IDC_CROSS);
        wndcls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wndcls.hInstance = g_hinst;
        wndcls.lpfnWndProc = WndProc;
        wndcls.lpszClassName = CLASS_NAME_DLL;
        wndcls.lpszMenuName = NULL;
        wndcls.style = CS_HREDRAW | CS_VREDRAW;
        RegisterClassA(&wndcls);
        /*
        wndcls.cbClsExtra = 0;
        wndcls.cbWndExtra = 0;
        wndcls.lpfnWndProc = WndProc;
        wndcls.hInstance = g_hinst;
        wndcls.lpszClassName = CLASS_NAME_DLL;
        wndcls.hCursor = LoadCursor(g_hinst, IDC_ARROW);
        wndcls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wndcls.style = CS_HREDRAW | CS_VREDRAW;
        if ((RegisterClassA(&wndcls)) == NULL)
        {
            return false;
        }*/
        // create window
        g_hwnd = CreateWindowExA(0, CLASS_NAME_DLL, CLASS_NAME_DLL, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, g_hinst, 0);
        if (g_hwnd == NULL)
        {
            return false;
        }

        UpdateWindow(g_hwnd);

        apihook::gdi_base::EnableHook();

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

        apihook::gdi_base::MyStacks_base::Inst().Clear();
        apihook::gdi_dc::MyStacks_relasedc::Inst().Clear();
        apihook::gdi_dc::MyStacks_deletedc::Inst().Clear();

        apihook::memory_heap::DisableHook();

        if (g_hwnd != NULL){
            DestroyWindow(g_hwnd);
            g_hwnd = NULL;
        }
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
        apihook::gdi_base::MyStacks_base::Inst().Clear();
        apihook::gdi_dc::MyStacks_relasedc::Inst().Clear();
        apihook::gdi_dc::MyStacks_deletedc::Inst().Clear();
    }
    static void dump()
    {
        std::string dlldir = GetModuleDir(g_hinst);

        HMODULE hProcess = GetModuleHandle(NULL);
        std::string proname = GetModuleName(hProcess);
        dlldir += "\\";
        dlldir += proname;
        dlldir += "_";

        apihook::gdi_base::MyStacks_base::Inst().Dump(dlldir + "base.leak");
        apihook::gdi_dc::MyStacks_relasedc::Inst().Dump(dlldir + "releasedc.leak");
        apihook::gdi_dc::MyStacks_deletedc::Inst().Dump(dlldir + "deletedc.leak");

        apihook::memory_heap::MyStacksIPs_memory::Inst().Dump(dlldir + "memory.leak");
    }

    static void me(int value)
    {
        apihook::memory_heap::EnableHook(value);
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
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (nMsg)
        {
        case WM_IPC_TODLL:
            HandleIPC(wParam, lParam);
            break;
        default:
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
        }

        return 0;
    }
};
