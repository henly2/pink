#pragma once

#include "BaseHook.hpp"

namespace apihook{
    namespace gdi_font{

#define ADDHOBJ(hobj) apihook::gdi_base::MyStacks_base::Inst().Add("font", (int)hobj);

        //////////////////////////////////////////////////////////////////////////
        // 定义类型和对象
        DEFINE_HOOKAPI(CreateFontA);
        DEFINE_HOOKAPI(CreateFontW);
        DEFINE_HOOKAPI(CreateFontIndirectA);
        DEFINE_HOOKAPI(CreateFontIndirectW);
        DEFINE_HOOKAPI(CreateFontIndirectExA);
        DEFINE_HOOKAPI(CreateFontIndirectExW);

        //////////////////////////////////////////////////////////////////////////
        // 自定义方法
        static HFONT WINAPI _PFN_CreateFontA(
            _In_ int     nHeight,
            _In_ int     nWidth,
            _In_ int     nEscapement,
            _In_ int     nOrientation,
            _In_ int     fnWeight,
            _In_ DWORD   fdwItalic,
            _In_ DWORD   fdwUnderline,
            _In_ DWORD   fdwStrikeOut,
            _In_ DWORD   fdwCharSet,
            _In_ DWORD   fdwOutputPrecision,
            _In_ DWORD   fdwClipPrecision,
            _In_ DWORD   fdwQuality,
            _In_ DWORD   fdwPitchAndFamily,
            _In_ LPCSTR lpszFace
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateFontA);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(nHeight, nWidth, nEscapement,
                nOrientation, fnWeight, fdwItalic, fdwUnderline,
                fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision,
                fdwQuality, fdwPitchAndFamily, lpszFace);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        static HFONT WINAPI _PFN_CreateFontW(
            _In_ int     nHeight,
            _In_ int     nWidth,
            _In_ int     nEscapement,
            _In_ int     nOrientation,
            _In_ int     fnWeight,
            _In_ DWORD   fdwItalic,
            _In_ DWORD   fdwUnderline,
            _In_ DWORD   fdwStrikeOut,
            _In_ DWORD   fdwCharSet,
            _In_ DWORD   fdwOutputPrecision,
            _In_ DWORD   fdwClipPrecision,
            _In_ DWORD   fdwQuality,
            _In_ DWORD   fdwPitchAndFamily,
            _In_ LPCWSTR lpszFace
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateFontW);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(nHeight, nWidth, nEscapement,
                nOrientation, fnWeight, fdwItalic, fdwUnderline,
                fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision,
                fdwQuality, fdwPitchAndFamily, lpszFace);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        static HFONT WINAPI _PFN_CreateFontIndirectA(__in CONST LOGFONTA *lplf)
        {
            auto& hookapi = GET_HOOKAPI(CreateFontIndirectA);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lplf);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }
        static HFONT WINAPI _PFN_CreateFontIndirectW(__in CONST LOGFONTW *lplf)
        {
            auto& hookapi = GET_HOOKAPI(CreateFontIndirectW);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lplf);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        static HFONT WINAPI _PFN_CreateFontIndirectExA(
            _In_ const ENUMLOGFONTEXDVA *penumlfex
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateFontIndirectExA);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(penumlfex);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        static HFONT WINAPI _PFN_CreateFontIndirectExW(
            _In_ const ENUMLOGFONTEXDVW *penumlfex
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateFontIndirectExW);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(penumlfex);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        //////////////////////////////////////////////////////////////////////////
        static void EnableHook()
        {
            ENABLE_HOOKAPI("gdi32.dll", CreateFontA);
            ENABLE_HOOKAPI("gdi32.dll", CreateFontW);
            ENABLE_HOOKAPI("gdi32.dll", CreateFontIndirectA);
            ENABLE_HOOKAPI("gdi32.dll", CreateFontIndirectW);

            ENABLE_HOOKAPI("gdi32.dll", CreateFontIndirectExA);
            ENABLE_HOOKAPI("gdi32.dll", CreateFontIndirectExW);
        }

        static void DisableHook()
        {
            DISABLE_HOOKAPI(CreateFontA);
            DISABLE_HOOKAPI(CreateFontW);
            DISABLE_HOOKAPI(CreateFontIndirectA);
            DISABLE_HOOKAPI(CreateFontIndirectW);
            DISABLE_HOOKAPI(CreateFontIndirectExA);
            DISABLE_HOOKAPI(CreateFontIndirectExW);
        }
    }
}
