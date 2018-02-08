#pragma once

#include "BaseHook.hpp"

namespace apihook{
    namespace gdi_extpen{

#define ADDHOBJ(hobj) apihook::gdi_base::MyStacks_base::Inst().Add("extpen", (int)hobj);

        //////////////////////////////////////////////////////////////////////////
        // 定义类型和对象
        DEFINE_HOOKAPI(ExtCreatePen);

        //////////////////////////////////////////////////////////////////////////
        // 自定义方法
        static HPEN WINAPI _PFN_ExtCreatePen(
            _In_       DWORD    dwPenStyle,
            _In_       DWORD    dwWidth,
            _In_ const LOGBRUSH *lplb,
            _In_       DWORD    dwStyleCount,
            _In_ const DWORD    *lpStyle
            )
        {
            auto& hookapi = GET_HOOKAPI(ExtCreatePen);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(dwPenStyle, dwWidth, lplb, dwStyleCount, lpStyle);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        //////////////////////////////////////////////////////////////////////////
        static void EnableHook()
        {
            ENABLE_HOOKAPI("gdi32.dll", ExtCreatePen);
        }

        static void DisableHook()
        {
            DISABLE_HOOKAPI(ExtCreatePen);
        }
    }
}
