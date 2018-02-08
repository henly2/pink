#pragma once

#include "BaseHook.hpp"

namespace apihook{
    namespace gdi_region{

#define ADDHOBJ(hobj) apihook::gdi_base::MyStacks_base::Inst().Add("region", (int)hobj);

        //////////////////////////////////////////////////////////////////////////
        // 定义类型和对象
        DEFINE_HOOKAPI(CreateEllipticRgn);
        DEFINE_HOOKAPI(CreateEllipticRgnIndirect);
        DEFINE_HOOKAPI(CreatePolygonRgn);
        DEFINE_HOOKAPI(CreatePolyPolygonRgn);
        DEFINE_HOOKAPI(CreateRectRgnIndirect);
        DEFINE_HOOKAPI(CreateRoundRectRgn);
        DEFINE_HOOKAPI(ExtCreateRegion);

        //////////////////////////////////////////////////////////////////////////
        // 自定义方法
        static HRGN WINAPI _PFN_CreateEllipticRgn(
            _In_ int nLeftRect,
            _In_ int nTopRect,
            _In_ int nRightRect,
            _In_ int nBottomRect
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateEllipticRgn);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(nLeftRect, nTopRect, nRightRect, nBottomRect);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }
        static HRGN WINAPI _PFN_CreateEllipticRgnIndirect(
            _In_ const RECT *lprc
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateEllipticRgnIndirect);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lprc);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }
        static HRGN WINAPI _PFN_CreatePolygonRgn(
            _In_ const POINT *lppt,
            _In_       int   cPoints,
            _In_       int   fnPolyFillMode
            )
        {
            auto& hookapi = GET_HOOKAPI(CreatePolygonRgn);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lppt, cPoints, fnPolyFillMode);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        static HRGN WINAPI _PFN_CreatePolyPolygonRgn(
            _In_ const POINT *lppt,
            _In_ const INT   *lpPolyCounts,
            _In_       int   nCount,
            _In_       int   fnPolyFillMode
            )
        {
            auto& hookapi = GET_HOOKAPI(CreatePolyPolygonRgn);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lppt, lpPolyCounts, nCount, fnPolyFillMode);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        static HRGN WINAPI _PFN_CreateRectRgnIndirect(
            _In_ const RECT *lprc
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateRectRgnIndirect);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lprc);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        static HRGN WINAPI _PFN_CreateRoundRectRgn(
            _In_ int nLeftRect,
            _In_ int nTopRect,
            _In_ int nRightRect,
            _In_ int nBottomRect,
            _In_ int nWidthEllipse,
            _In_ int nHeightEllipse
            )
        {
            auto& hookapi = GET_HOOKAPI(CreateRoundRectRgn);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(nLeftRect, nTopRect, nRightRect, nBottomRect, nWidthEllipse, nHeightEllipse);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        static HRGN WINAPI _PFN_ExtCreateRegion(
            _In_ const XFORM   *lpXform,
            _In_       DWORD   nCount,
            _In_ const RGNDATA *lpRgnData
            )
        {
            auto& hookapi = GET_HOOKAPI(ExtCreateRegion);

            hookapi.CallBefore();
            auto hobj = hookapi.hookfunc(lpXform, nCount, lpRgnData);
            hookapi.CallAfter();

            ADDHOBJ(hobj);

            return hobj;
        }

        //////////////////////////////////////////////////////////////////////////
        static void EnableHook()
        {
            ENABLE_HOOKAPI("gdi32.dll", CreateEllipticRgn);
            ENABLE_HOOKAPI("gdi32.dll", CreateEllipticRgnIndirect);
            ENABLE_HOOKAPI("gdi32.dll", CreatePolygonRgn);
            ENABLE_HOOKAPI("gdi32.dll", CreatePolyPolygonRgn);
            ENABLE_HOOKAPI("gdi32.dll", CreateRectRgnIndirect);
            ENABLE_HOOKAPI("gdi32.dll", CreateRoundRectRgn);
            ENABLE_HOOKAPI("gdi32.dll", ExtCreateRegion);
        }

        static void DisableHook()
        {
            DISABLE_HOOKAPI(CreateEllipticRgn);
            DISABLE_HOOKAPI(CreateEllipticRgnIndirect);
            DISABLE_HOOKAPI(CreatePolygonRgn);
            DISABLE_HOOKAPI(CreatePolyPolygonRgn);
            DISABLE_HOOKAPI(CreateRectRgnIndirect);
            DISABLE_HOOKAPI(CreateRoundRectRgn);
            DISABLE_HOOKAPI(ExtCreateRegion);
        }
    }
}
