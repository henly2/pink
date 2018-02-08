#pragma once

#include "BaseHook.hpp"

namespace apihook{
    namespace gdi_brush{

#define ADDHOBJ(hobj) apihook::gdi_base::MyStacks_base::Inst().Add("brush", (int)hobj);

        //////////////////////////////////////////////////////////////////////////
        // 定义类型和对象
        DEFINE_HOOKAPI(CreateSolidBrush);
        DEFINE_HOOKAPI(CreateBrushIndirect);
        DEFINE_HOOKAPI(CreateDIBPatternBrushPt);
        DEFINE_HOOKAPI(CreateHatchBrush);
        DEFINE_HOOKAPI(CreatePatternBrush);

        //////////////////////////////////////////////////////////////////////////
        // 自定义方法
        static HBRUSH WINAPI _PFN_CreateSolidBrush(
            _In_ COLORREF crColor
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateSolidBrush);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(crColor);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }
        static HBRUSH WINAPI _PFN_CreateBrushIndirect(
            _In_ const LOGBRUSH *lplb
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateBrushIndirect);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lplb);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }
        static HBRUSH WINAPI _PFN_CreateDIBPatternBrushPt(
            _In_ const VOID *lpPackedDIB,
            _In_       UINT iUsage
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateDIBPatternBrushPt);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lpPackedDIB, iUsage);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }
        static HBRUSH WINAPI _PFN_CreateHatchBrush(
            _In_ int      fnStyle,
            _In_ COLORREF clrref
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateHatchBrush);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(fnStyle, clrref);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }
        static HBRUSH WINAPI _PFN_CreatePatternBrush(
            _In_ HBITMAP hbmp
            )
        {
            auto& hookapi = GET_HOOKAPI(CreatePatternBrush);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(hbmp);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        //////////////////////////////////////////////////////////////////////////
        static void EnableHook()
        {
            ENABLE_HOOKAPI("gdi32.dll", CreateSolidBrush);
            ENABLE_HOOKAPI("gdi32.dll", CreateBrushIndirect);
            ENABLE_HOOKAPI("gdi32.dll", CreateDIBPatternBrushPt);
            ENABLE_HOOKAPI("gdi32.dll", CreateHatchBrush);
            ENABLE_HOOKAPI("gdi32.dll", CreatePatternBrush);
        }

        static void DisableHook()
        {
            DISABLE_HOOKAPI(CreateSolidBrush);
            DISABLE_HOOKAPI(CreateBrushIndirect);
            DISABLE_HOOKAPI(CreateDIBPatternBrushPt);
            DISABLE_HOOKAPI(CreateHatchBrush);
            DISABLE_HOOKAPI(CreatePatternBrush);
        }
    }
}
