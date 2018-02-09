#pragma once

#include <Windows.h>

#include <string>

#include "../source/apihook/APIHook.hpp"
#include "../source/apihook/Gdi/Gdi.hpp"

#include "../Hooker/common.h"

using namespace hook;

HINSTANCE hinst_ = NULL;
HWND hwnd_ = NULL;
std::string tips_;
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
        hinst_ = hinstance;

        // register class
        WNDCLASSEXA wex = { 0 };
        wex.cbSize = sizeof(WNDCLASSEXA);

        wex.style = CS_HREDRAW | CS_VREDRAW;
       
        wex.cbClsExtra = 0;
        wex.cbWndExtra = 0;
        wex.lpfnWndProc = WndProc;
        wex.hInstance = hinst_;
        wex.lpszClassName = CLASS_NAME;
        wex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        if ((RegisterClassExA(&wex)) == NULL)
        {
            return false;
        }
        // create window
        hwnd_ = CreateWindowA(CLASS_NAME, NULL, WS_POPUP|WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            parent, NULL, hinst_, NULL);
        if (hwnd_ == NULL)
        {
            return false;
        }

        int w = 200, h = 50;
        HWND hd = GetDesktopWindow();
        RECT rt;
        GetClientRect(hd, &rt);
        int l = rt.right / 2 - w / 2;
        int t = 0;
        SetWindowPos(hwnd_, HWND_TOPMOST, l, t, w, h, SWP_SHOWWINDOW);
        UpdateWindow(hwnd_);

        apihook::StackWalker::Inst().Enable();
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
        // 不在相同的线程enable和diable，会崩溃
        //apihook::StackWalker::Inst().Disable();

        apihook::gdi_base::MyStacks_base::Inst().Clear();
        apihook::gdi_dc::MyStacks_relasedc::Inst().Clear();
        apihook::gdi_dc::MyStacks_deletedc::Inst().Clear();
    }

    // 获取窗口
    HWND GetHWND() const
    {
        return hwnd_;
    }

private:
#define HE_IS_GDIID(value, item) if (IS_GDIID(value, item))   \
        { \
            apihook::gdi_##item::EnableHook();\
            tips_ = "enable ";\
            tips_ += #item;\
            InvalidateRect(hwnd_, NULL, TRUE);\
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
            tips_ = "disable ";\
            tips_ += #item;\
            InvalidateRect(hwnd_, NULL, TRUE);\
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
        tips_ = "clear...";
        apihook::gdi_base::MyStacks_base::Inst().Clear();
        apihook::gdi_dc::MyStacks_relasedc::Inst().Clear();
        apihook::gdi_dc::MyStacks_deletedc::Inst().Clear();

        tips_ = "clear done...";
        InvalidateRect(hwnd_, NULL, TRUE);
    }
    static void dump()
    {
        tips_ = "dump...";
        InvalidateRect(hwnd_, NULL, TRUE);

        apihook::gdi_base::MyStacks_base::Inst().Dump("base.leak");
        apihook::gdi_dc::MyStacks_relasedc::Inst().Dump("releasedc.leak");
        apihook::gdi_dc::MyStacks_deletedc::Inst().Dump("deletedc.leak");

        tips_ = "dump done...";
        InvalidateRect(hwnd_, NULL, TRUE);
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
        default:
            break;
        }

        InvalidateRect(hwnd_, NULL, TRUE);
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
    {
        int wmId, wmEvent;
        PAINTSTRUCT ps;
        HDC hdc;

        switch (nMsg)
        {
        case WM_IPC:
            HandleIPC(wParam, lParam);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            // TODO:  在此添加任意绘图代码...
            {
                std::string text = "In ";
                text += std::to_string(GetCurrentProcessId());

                RECT rt;
                GetClientRect(hWnd, &rt);
                rt.bottom /= 2;
                DrawTextA(hdc, text.c_str(), text.length(), &rt, 0);

                rt.top = rt.bottom;
                rt.bottom *= 2;
                DrawTextA(hdc, tips_.c_str(), tips_.length(), &rt, 0);
            }
            EndPaint(hWnd, &ps);
            break;
        default:
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
        }

        return 0;
    }
};
