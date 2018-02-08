#pragma once

#include "../APIHook.hpp"

namespace apihook{
    namespace gdi_base{
        //////////////////////////////////////////////////////////////////////////
        // 定义全局记录
        DEFINE_MYSTACK_INST(base);

        //////////////////////////////////////////////////////////////////////////
        // 定义类型
        typedef decltype(DeleteObject) (*PFN_DeleteObject);

        //////////////////////////////////////////////////////////////////////////
        // 定义对象
        static APIHookInfo<PFN_DeleteObject> s_PFN_DeleteObject;

        //////////////////////////////////////////////////////////////////////////
        // 自定义方法
        static BOOL WINAPI _PFN_DeleteObject(
            _In_ HGDIOBJ hObject
            )
        {
            s_PFN_DeleteObject.CallBefore();
            BOOL ret = s_PFN_DeleteObject.hookfunc(hObject);
            s_PFN_DeleteObject.CallAfter();

            MyStacks_base::Inst().Remove((int)hObject);

            return ret;
        }

        //////////////////////////////////////////////////////////////////////////
        static void EnableHook()
        {
            s_PFN_DeleteObject.Hook("gdi32.dll", "DeleteObject", _PFN_DeleteObject);
        }

        static void DisableHook()
        {
            s_PFN_DeleteObject.UnHook();
        }
    }
}
