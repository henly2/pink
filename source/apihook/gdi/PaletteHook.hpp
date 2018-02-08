#pragma once

#include "BaseHook.hpp"

namespace apihook{
    namespace gdi_palette{

#define ADDHOBJ(hobj) apihook::gdi_base::MyStacks_base::Inst().Add("palette", (int)hobj);

        //////////////////////////////////////////////////////////////////////////
        // 定义类型和对象
        DEFINE_HOOKAPI(CreateHalftonePalette);
        DEFINE_HOOKAPI(CreatePalette);

        //////////////////////////////////////////////////////////////////////////
        // 自定义方法
        static HPALETTE WINAPI _PFN_CreateHalftonePalette(
            _In_ HDC hdc
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateHalftonePalette);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(hdc);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }
        static HPALETTE WINAPI _PFN_CreatePalette(
            _In_ const LOGPALETTE *lplgpl
            )
        {
            auto& hookapi = GET_HOOKAPI(CreatePalette);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lplgpl);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        //////////////////////////////////////////////////////////////////////////
        static void EnableHook()
        {
            ENABLE_HOOKAPI("gdi32.dll", CreateHalftonePalette);
            ENABLE_HOOKAPI("gdi32.dll", CreatePalette);
        }

        static void DisableHook()
        {
            DISABLE_HOOKAPI(CreateHalftonePalette);
            DISABLE_HOOKAPI(CreatePalette);
        }
    }
}
