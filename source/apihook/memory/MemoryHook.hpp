#pragma once

#include "../APIHook2.hpp"

namespace apihook{
    namespace memory_heap{
        //////////////////////////////////////////////////////////////////////////
        // 定义全局记录
        DEFINE_MYSTACK_INST(memory);

        //////////////////////////////////////////////////////////////////////////
        // 定义类型和对象
        DEFINE_HOOKAPI2(HeapAlloc);
        DEFINE_HOOKAPI2(HeapFree);

        //////////////////////////////////////////////////////////////////////////
        // 自定义方法
        static LPVOID WINAPI _PFN_HeapAlloc(
            _In_ HANDLE hHeap,
            _In_ DWORD  dwFlags,
            _In_ SIZE_T dwBytes
            )
        {
            auto data = s_PFN2_HeapAlloc.hookfunc(hHeap, dwFlags, dwBytes);

            StackWalkerIPC::ContextIPC cs;
            cs.type = 1;
            cs.addr = (int)data;
            cs.size = dwBytes;     

            StackWalkerIPC::Inst().NotifyRemoteWalker(cs);
            
            return data;
        }

        static BOOL WINAPI _PFN_HeapFree(
            _In_ HANDLE hHeap,
            _In_ DWORD  dwFlags,
            _In_ LPVOID lpMem
            )
        {
            auto data = s_PFN2_HeapFree.hookfunc(hHeap, dwFlags, lpMem);

            StackWalkerIPC::ContextIPC cs;
            cs.type = -1;
            cs.addr = (int)lpMem;
            cs.size = 0;

            StackWalkerIPC::Inst().NotifyRemoteWalker(cs);

            return data;
        }


        //////////////////////////////////////////////////////////////////////////
        static void EnableHook()
        {
            ENABLE_HOOKAPI2(HeapFree, _PFN_HeapFree);
            ENABLE_HOOKAPI2(HeapAlloc, _PFN_HeapAlloc);
        }

        static void DisableHook()
        {
            DISABLE_HOOKAPI2(HeapAlloc);
            DISABLE_HOOKAPI2(HeapFree);
        }
    }
}
