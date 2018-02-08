#pragma once

#include "BaseHook.hpp"

namespace apihook{
    namespace gdi_dc{

        DEFINE_MYSTACK_INST(deletedc);
        DEFINE_MYSTACK_INST(relasedc);
#define GET_deletedc() MyStacks_deletedc::Inst()
#define GET_releasedc() MyStacks_relasedc::Inst()

        //////////////////////////////////////////////////////////////////////////
        // 定义类型和对象
        DEFINE_HOOKAPI(CreateCompatibleDC);
        DEFINE_HOOKAPI(CreateDC);
        DEFINE_HOOKAPI(CreateIC);
        DEFINE_HOOKAPI(DeleteDC);

        DEFINE_HOOKAPI(GetDC);
        DEFINE_HOOKAPI(GetDCEx);
        DEFINE_HOOKAPI(ReleaseDC);

        //////////////////////////////////////////////////////////////////////////
        // 自定义方法
        static HDC WINAPI _PFN_CreateCompatibleDC(
            _In_ HDC hdc
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateCompatibleDC);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(hdc);
            hookapi.CallAfter();

            GET_deletedc().Add("dc", (int)hobj);

            return hobj;
        }
        static HDC WINAPI _PFN_CreateDC(
            LPCTSTR lpszDriver,
            _In_       LPCTSTR lpszDevice,
            LPCTSTR lpszOutput,
            _In_ const DEVMODE *lpInitData
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateDC);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lpszDriver, lpszDevice, lpszOutput, lpInitData);
            hookapi.CallAfter();

            GET_deletedc().Add("dc", (int)hobj);

            return hobj;
        }
        static HDC WINAPI _PFN_CreateIC(
            _In_       LPCTSTR lpszDriver,
            _In_       LPCTSTR lpszDevice,
            LPCTSTR lpszOutput,
            _In_ const DEVMODE *lpdvmInit
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateIC);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lpszDriver, lpszDevice, lpszOutput, lpdvmInit);
            hookapi.CallAfter();

            GET_deletedc().Add("dc", (int)hobj);

            return hobj;
        }
        static BOOL WINAPI _PFN_DeleteDC(
            _In_ HDC hdc
            )
        {
            auto& hookapi = GET_HOOKAPI(DeleteDC);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(hdc);
            hookapi.CallAfter();

            GET_deletedc().Remove((int)hdc);

            return hobj;
        }

        //////////////////////////////////////////////////////////////////////////
        static HDC WINAPI _PFN_GetDC(
            _In_ HWND hWnd
            )
        {
            auto& hookapi = GET_HOOKAPI(GetDC);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(hWnd);
            hookapi.CallAfter();

            GET_releasedc().Add("dc", (int)hobj);

            return hobj;
        }

        static HDC WINAPI _PFN_GetDCEx(
            _In_ HWND  hWnd,
            _In_ HRGN  hrgnClip,
            _In_ DWORD flags
            )
        {
            auto& hookapi = GET_HOOKAPI(GetDCEx);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(hWnd, hrgnClip, flags);
            hookapi.CallAfter();

            GET_releasedc().Add("dc", (int)hobj);

            return hobj;
        }

        static int WINAPI _PFN_ReleaseDC(
            _In_ HWND hWnd,
            _In_ HDC  hDC
            )
        {
            auto& hookapi = GET_HOOKAPI(ReleaseDC);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(hWnd, hDC);
            hookapi.CallAfter();

            GET_releasedc().Remove((int)hDC);

            return hobj;
        }

        //////////////////////////////////////////////////////////////////////////
        static void EnableHook()
        {
            ENABLE_HOOKAPI("gdi32.dll", CreateCompatibleDC);
            ENABLE_HOOKAPI("gdi32.dll", CreateDC);
            ENABLE_HOOKAPI("gdi32.dll", CreateIC);
            ENABLE_HOOKAPI("gdi32.dll", DeleteDC);

            ENABLE_HOOKAPI("user32.dll", GetDC);
            ENABLE_HOOKAPI("user32.dll", GetDCEx);
            ENABLE_HOOKAPI("user32.dll", ReleaseDC);
        }

        static void DisableHook()
        {
            DISABLE_HOOKAPI(CreateCompatibleDC);
            DISABLE_HOOKAPI(CreateDC);
            DISABLE_HOOKAPI(CreateIC);
            DISABLE_HOOKAPI(DeleteDC);

            DISABLE_HOOKAPI(GetDC);
            DISABLE_HOOKAPI(GetDCEx);
            DISABLE_HOOKAPI(ReleaseDC);
        }
    }
}
