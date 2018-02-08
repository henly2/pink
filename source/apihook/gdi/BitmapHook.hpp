#pragma once

#include "BaseHook.hpp"

namespace apihook{
    namespace gdi_bitmap{

#define ADDHOBJ(hobj) apihook::gdi_base::MyStacks_base::Inst().Add("bitmap", (int)hobj);

        //////////////////////////////////////////////////////////////////////////
        // 定义类型和对象
        DEFINE_HOOKAPI(CreateBitmap);
        DEFINE_HOOKAPI(CreateBitmapIndirect);
        DEFINE_HOOKAPI(CreateCompatibleBitmap);
        DEFINE_HOOKAPI(CreateDIBitmap);
        DEFINE_HOOKAPI(CreateDIBSection);
        DEFINE_HOOKAPI(LoadBitmap);

        //////////////////////////////////////////////////////////////////////////
        // 自定义方法
        static HBITMAP WINAPI _PFN_CreateBitmap(
            _In_       int  nWidth,
            _In_       int  nHeight,
            _In_       UINT cPlanes,
            _In_       UINT cBitsPerPel,
            _In_ const VOID *lpvBits
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateBitmap);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(nWidth, nHeight, cPlanes, cBitsPerPel, lpvBits);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }
        static HBITMAP WINAPI _PFN_CreateBitmapIndirect(
            _In_ const BITMAP *lpbm
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateBitmapIndirect);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lpbm);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }
        static HBITMAP WINAPI _PFN_CreateCompatibleBitmap(
            _In_ HDC hdc,
            _In_ int nWidth,
            _In_ int nHeight
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateCompatibleBitmap);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(hdc, nWidth, nHeight);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }
        static HBITMAP WINAPI _PFN_CreateDIBitmap(
            _In_       HDC              hdc,
            _In_ const BITMAPINFOHEADER *lpbmih,
            _In_       DWORD            fdwInit,
            _In_ const VOID             *lpbInit,
            _In_ const BITMAPINFO       *lpbmi,
            _In_       UINT             fuUsage
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateDIBitmap);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(hdc, lpbmih, fdwInit, lpbInit, lpbmi, fuUsage);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        static HBITMAP WINAPI _PFN_CreateDIBSection(
            _In_        HDC        hdc,
            _In_  const BITMAPINFO *pbmi,
            _In_        UINT       iUsage,
            _Out_       VOID       **ppvBits,
            _In_        HANDLE     hSection,
            _In_        DWORD      dwOffset
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateDIBSection);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(hdc, pbmi, iUsage, ppvBits, hSection, dwOffset);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        static HBITMAP WINAPI _PFN_LoadBitmap(
            _In_ HINSTANCE hInstance,
            _In_ LPCTSTR   lpBitmapName
            )
        {
            auto& hookapi = GET_HOOKAPI(LoadBitmap);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(hInstance, lpBitmapName);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        //////////////////////////////////////////////////////////////////////////
        static void EnableHook()
        {
            ENABLE_HOOKAPI("gdi32.dll", CreateBitmap);
            ENABLE_HOOKAPI("gdi32.dll", CreateBitmapIndirect);
            ENABLE_HOOKAPI("gdi32.dll", CreateCompatibleBitmap);
            ENABLE_HOOKAPI("gdi32.dll", CreateDIBitmap);
            ENABLE_HOOKAPI("gdi32.dll", CreateDIBSection);
            ENABLE_HOOKAPI("gdi32.dll", LoadBitmap);
        }

        static void DisableHook()
        {
            DISABLE_HOOKAPI(CreateBitmap);
            DISABLE_HOOKAPI(CreateBitmapIndirect);
            DISABLE_HOOKAPI(CreateCompatibleBitmap);
            DISABLE_HOOKAPI(CreateDIBitmap);
            DISABLE_HOOKAPI(CreateDIBSection);
            DISABLE_HOOKAPI(LoadBitmap);
        }
    }
}
