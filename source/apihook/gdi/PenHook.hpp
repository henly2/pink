#pragma once

#include "BaseHook.hpp"

namespace apihook{
    namespace gdi_pen{

#define ADDHOBJ(hobj) apihook::gdi_base::MyStacks_base::Inst().Add("pen", (int)hobj);

        //////////////////////////////////////////////////////////////////////////
        // 定义类型和对象
        DEFINE_HOOKAPI(CreatePen);
        DEFINE_HOOKAPI(CreatePenIndirect);

        //////////////////////////////////////////////////////////////////////////
        // 自定义方法
        static HPEN WINAPI _PFN_CreatePen(_In_ int iStyle, _In_ int cWidth, _In_ COLORREF color)
        {
            auto& hookapi = GET_HOOKAPI(CreatePen);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(iStyle, cWidth, color);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }
        static HPEN WINAPI _PFN_CreatePenIndirect(_In_ CONST LOGPEN *plpen)
        {
            auto& hookapi = GET_HOOKAPI(CreatePenIndirect);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(plpen);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        //////////////////////////////////////////////////////////////////////////
        static void EnableHook()
        {
            ENABLE_HOOKAPI("gdi32.dll", CreatePen);
            ENABLE_HOOKAPI("gdi32.dll", CreatePenIndirect);
        }

        static void DisableHook()
        {
            DISABLE_HOOKAPI(CreatePen);
            DISABLE_HOOKAPI(CreatePenIndirect);
        }
    }
}
