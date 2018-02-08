#pragma once

#include "../APIHook.hpp"

namespace apihook{
    namespace gdi_base{
        //////////////////////////////////////////////////////////////////////////
        // ����ȫ�ּ�¼
        DEFINE_MYSTACK_INST(base);

        //////////////////////////////////////////////////////////////////////////
        // ��������
        typedef decltype(DeleteObject) (*PFN_DeleteObject);

        //////////////////////////////////////////////////////////////////////////
        // �������
        static APIHookInfo<PFN_DeleteObject> s_PFN_DeleteObject;

        //////////////////////////////////////////////////////////////////////////
        // �Զ��巽��
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
