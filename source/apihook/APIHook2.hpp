#pragma once

#include <sstream>
#include <fstream>
#include <string>
#include <unordered_map>

#include "APIHook.hpp"

// MS Detours
#include "E:/test_workspace/third/Detours Version 3.0 Build_343/Detours/include/detours.h"
#include "E:/test_workspace/third/Detours Version 3.0 Build_343/Detours/include/syelog.h"
#pragma comment (lib, "E:/test_workspace/third/Detours Version 3.0 Build_343/Detours/lib.X86/detours.lib")
#pragma comment (lib, "E:/test_workspace/third/Detours Version 3.0 Build_343/Detours/lib.X86/syelog.lib")

// StackWalker from https://github.com/JochenKalmbach/StackWalker
#include "E:/test_workspace/third/StackWalker/Main/StackWalker/StackWalker.h"
class StackWalkerToConsole : public StackWalker
{
public:
    StackWalkerToConsole(DWORD dwProcessId, HANDLE hProcess)
        :StackWalker(dwProcessId, hProcess)
    {
    }

    std::string out;

protected:
    virtual void OnOutput(LPCSTR szText){
        out += szText;
    }
};

//////////////////////////////////////////////////////////////////////////
namespace apihook {
    //////////////////////////////////////////////////////////////////////////
    template <typename FUNC>
    class APIHookInfo2
    {
    public:
        FUNC hookfunc;
        FUNC yourfunc;

        APIHookInfo2(){
            memset(this, 0, sizeof(APIHookInfo2));
        }

        long Hook(FUNC _hookfunc, FUNC _yourfunc){
            hookfunc = _hookfunc;
            yourfunc = _yourfunc;

            DetourRestoreAfterWith();
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            DetourAttach(&(PVOID&)hookfunc, yourfunc);
            return DetourTransactionCommit();
        }

        long UnHook()
        {
            if (hookfunc==NULL && yourfunc==NULL){
                return 0;
            }

            DetourRestoreAfterWith();
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            DetourDetach(&(PVOID&)hookfunc, yourfunc);
            return DetourTransactionCommit();
        }
    };

#define DEFINE_HOOKAPI2(real) \
    typedef decltype(real) (*PFN2_##real);\
    static APIHookInfo2<PFN2_##real> s_PFN2_##real;

#define GET_HOOKAPI2(real) s_PFN2_##real

#define ENABLE_HOOKAPI2(real, your) s_PFN2_##real.Hook(real, your);
#define DISABLE_HOOKAPI2(real) s_PFN2_##real.UnHook();

    //////////////////////////////////////////////////////////////////////////
    class StackWalkerIPC
    {
    public:
        static StackWalkerIPC& Inst()
        {
            static StackWalkerIPC s;
            return s;
        }

        // 本地
        bool EnableLocal(const char* classname, unsigned int msg)
        {
            classname_ = classname;
            msg_ = msg;
            hwnd_ = FindWindowA(classname_.c_str(), NULL);
            if (hwnd_ == NULL)
            {
                hwnd_ = FindWindowA(NULL, classname_.c_str());
            }
            if (hwnd_ == NULL)
            {
                return false;
            }

            return OpenSharedMemory();
        }

        void DisableLocal()
        {
            CloseSharedMemory();
        }

        // 远程调用
        bool EnableRemote(DWORD remotepid)
        {
            if (remotepid == pid_)
            {
                return true;
            }

            DisableRemote();

            pid_ = remotepid;
            if (pid_ == GetCurrentProcessId())
                hprocess_ = GetCurrentProcess();
            else
                hprocess_ = OpenProcess(PROCESS_ALL_ACCESS, 0, pid_);

            walker_ = new StackWalkerToConsole(pid_, hprocess_);

            return OpenSharedMemory();
        }

        void DisableRemote()
        {
            if (hprocess_ != NULL)
                CloseHandle(hprocess_);
            hprocess_ = NULL;

            pid_ = 0;

            CloseSharedMemory();

            if (walker_)
                delete walker_;
            walker_ = NULL;
        }

        struct ContextIPC
        {
            DWORD tid;

            int type; // 1: add, -1:remove
            int addr;
            int size;

            ContextIPC(){
                memset(this, 0, sizeof(ContextIPC));

                tid = GetCurrentThreadId();
            }
        };
        
        bool OpenSharedMemory()
        {
            if (hMap_ != NULL)
            {
                return true;
            }

            std::string strMapName("ContextRemote");                // 内存映射对象名称
            //pBuffer;                                // 共享内存指针

            // 首先试图打开一个命名的内存映射文件对象  
            hMap_ = ::OpenFileMappingA(FILE_MAP_ALL_ACCESS, 0, strMapName.c_str());
            if (NULL == hMap_)
            {    // 打开失败，创建之
                hMap_ = ::CreateFileMappingA(INVALID_HANDLE_VALUE,
                    NULL,
                    PAGE_READWRITE,
                    0,
                    sizeof(ContextIPC),
                    strMapName.c_str());
                // 映射对象的一个视图，得到指向共享内存的指针，设置里面的数据
                pBuffer_ = ::MapViewOfFile(hMap_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            }
            else
            {    // 打开成功，映射对象的一个视图，得到指向共享内存的指针，显示出里面的数据
                pBuffer_ = ::MapViewOfFile(hMap_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            }

            return true;
        }

        void CloseSharedMemory()
        {
            // 解除文件映射，关闭内存映射文件对象句柄
            if (pBuffer_)
                ::UnmapViewOfFile(pBuffer_);
            if (hMap_)
                ::CloseHandle(hMap_);

            pBuffer_ = NULL;
            hMap_ = NULL;
        }

        bool WriteSharedMemory(const ContextIPC& cs)
        {
            if (hMap_ == NULL || pBuffer_ == NULL)
            {
                return false;
            }

            memcpy(pBuffer_, &cs, sizeof(ContextIPC));
            return true;
        }

        bool ReadSharedMemory(ContextIPC& cs)
        {
            if (hMap_ == NULL || pBuffer_ == NULL)
            {
                return false;
            }

            memcpy(&cs, pBuffer_, sizeof(ContextIPC));
            return true;
        }

    public:
        LRESULT NotifyRemoteWalker(const ContextIPC& cs)
        {
            if (hwnd_ == NULL)
            {
                return -1;
            }
            ScopedLock lock(cs_lock_);

            StackWalkerIPC::Inst().WriteSharedMemory(cs);
            return ::SendMessage(hwnd_, msg_, (WPARAM)NULL, (LPARAM)NULL);
        }

        std::string WalkerRemote(const ContextIPC& cs)
        {
            HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, cs.tid);
            if (hThread == NULL){
                return "hThread is null";
            }
            
            walker_->out.clear();
            walker_->ShowSimpleCallstack(hThread);

            std::stringstream ss;
            ss << "\n";
            ss << "thread[" << cs.tid << "],addr[" << cs.addr << "],size[" << cs.size << "]\n";
            ss << walker_->out;
            ss << "\n";

            CloseHandle(hThread);

            return ss.str();
        }
    
        void SetFilterThread(int filterthread){
            filter_thread_ = filterthread;
        }

        bool IsFilterThread(){
            return filter_thread_ == 0 || GetCurrentThreadId() == filter_thread_;
        }
    private:
        StackWalkerIPC(void)
            : pid_(0)
            , hprocess_(NULL)
            , hMap_(NULL)
            , pBuffer_(NULL)
            , walker_(NULL)
            , hwnd_(NULL)
            , msg_(0)
            , filter_thread_(0)
        {

        }

        ~StackWalkerIPC(void)
        {

        }

    private:
        DWORD pid_;
        HANDLE hprocess_;

        int filter_thread_;


        HANDLE hEvent_;
        HANDLE hMap_;
        LPVOID pBuffer_;

        StackWalkerToConsole* walker_;

        CSLock cs_lock_;

        std::string classname_;
        HWND hwnd_;
        unsigned int msg_;
    };

}