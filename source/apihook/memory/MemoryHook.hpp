#pragma once

#include "../APIHook2.hpp"

namespace apihook{
    namespace memory_heap{
        //////////////////////////////////////////////////////////////////////////
        // 定义全局记录
        DEFINE_MYSTACKIPS_INST(memory);

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
            if (!StackWalkerIPC::Inst().IsFilterThread())
            {
                return data;
            }

            if (StackWalkerIPC::Inst().in_doing())
            {
                return data;
            }
            /*
            StackWalkerIPC::ContextIPC cs;
            cs.type = 1;
            cs.addr = (int)data;
            cs.size = dwBytes;     

            StackWalkerIPC::Inst().NotifyRemoteWalker(cs);
            */
            apihook::memory_heap::MyStacksIPs_memory::Inst().Add((int)data);
            StackWalkerIPC::Inst().out_doing();
            
            return data;
        }

        static BOOL WINAPI _PFN_HeapFree(
            _In_ HANDLE hHeap,
            _In_ DWORD  dwFlags,
            _In_ LPVOID lpMem
            )
        {
            auto data = s_PFN2_HeapFree.hookfunc(hHeap, dwFlags, lpMem);
            if (!StackWalkerIPC::Inst().IsFilterThread())
            {
                return data;
            }

            if (StackWalkerIPC::Inst().in_doing())
            {
                return data;
            }
            /*
            StackWalkerIPC::ContextIPC cs;
            cs.type = -1;
            cs.addr = (int)lpMem;
            cs.size = 0;

            StackWalkerIPC::Inst().NotifyRemoteWalker(cs);
            */

            apihook::memory_heap::MyStacksIPs_memory::Inst().Remove((int)lpMem);
            StackWalkerIPC::Inst().out_doing();
            return data;
        }


        //////////////////////////////////////////////////////////////////////////
        static void EnableHook(int filterthread = 0)
        {
            StackWalkerIPC::Inst().SetFilterThread(filterthread);

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
