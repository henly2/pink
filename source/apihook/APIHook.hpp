#pragma once

#include <sstream>
#include <fstream>
#include <string>
#include <unordered_map>

#include <Dbghelp.h>  
#include <DelayImp.h>  
#pragma comment( lib, "Dbghelp.lib")  
#define DBGHELP_TRANSLATE_TCHAR

namespace apihook {

    class CSLock{
        CRITICAL_SECTION	m_cs;
    public:
        CSLock(){ InitializeCriticalSection(&m_cs); }
        ~CSLock(){ DeleteCriticalSection(&m_cs); }
        void SetSpinCount(int count){ SetCriticalSectionSpinCount(&m_cs, count); }
        void Lock(){ EnterCriticalSection(&m_cs); }
        void Unlock(){ LeaveCriticalSection(&m_cs); }
    };

    class ScopedLock{
        CSLock&		m_lock;
    public:
        ScopedLock(CSLock& lock) :m_lock(lock){ m_lock.Lock(); }
        ~ScopedLock(){ m_lock.Unlock(); }
    };

//////////////////////////////////////////////////////////////////////////
template<typename FUNC>
class APIHookInfo
{
    FUNC yourfunc;

    unsigned char code[5];
    unsigned char oldcode[5];

    HANDLE process;

public:
	FUNC hookfunc;

    APIHookInfo(){
        memset(this, 0, sizeof(*this));
    }

    void Hook(const char* dllname, const char* funcname, FUNC yourfunc){
        if (process != NULL)
            return;

        process = GetCurrentProcess();

        HMODULE hdll = GetModuleHandleA(dllname);
        hookfunc = (FUNC)GetProcAddress(hdll, funcname);

        if (hookfunc){
            code[0] = 0xe9;
            DWORD a = (DWORD)yourfunc - (DWORD)hookfunc - 5;
            RtlMoveMemory(code + 1, &a, 4);

            DWORD old;
            if (VirtualProtectEx(process, hookfunc, 5, PAGE_EXECUTE_READWRITE, &old)){
                RtlMoveMemory(oldcode, hookfunc, 5);
                WriteProcessMemory(process, hookfunc, code, 5, NULL);
                VirtualProtectEx(process, hookfunc, 5, old, &old);
            }
        }
    }

	void UnHook()
	{
        if (process == NULL)
            return;

		CallBefore();

        CloseHandle(process);
        process = NULL;
	}

    void CallBefore()
    {
        DWORD old;
        if (VirtualProtectEx(process, hookfunc, 5, PAGE_EXECUTE_READWRITE, &old)){
            WriteProcessMemory(process, hookfunc, oldcode, 5, NULL);
            VirtualProtectEx(process, hookfunc, 5, old, &old);
        }
    }

    void CallAfter()
    {
        DWORD old;
        if (VirtualProtectEx(process, hookfunc, 5, PAGE_EXECUTE_READWRITE, &old)){
            WriteProcessMemory(process, hookfunc, code, 5, NULL);
            VirtualProtectEx(process, hookfunc, 5, old, &old);
        }
    }
};

#define DEFINE_HOOKAPI(api) \
    typedef decltype(api) (*PFN_##api);\
    static APIHookInfo<PFN_##api> s_PFN_##api;

#define DEF_YOUAPINAME() _PFN_

#define GET_HOOKAPI(api) s_PFN_##api

#define ENABLE_HOOKAPI(dll, api) s_PFN_##api.Hook(dll, #api, _PFN_##api);
#define DISABLE_HOOKAPI(api) s_PFN_##api.UnHook();

//////////////////////////////////////////////////////////////////////////
class StackWalker
{
public:
    StackWalker(void)
    {
    }

    ~StackWalker(void)
    {
    }

public:
    static std::string DumpFullStack(HANDLE _hprocess=NULL, HANDLE _hthread=NULL)
    {
        HANDLE hprocess = _hprocess;
        if (hprocess == NULL)
            hprocess = GetCurrentProcess();

        HANDLE hthread = _hthread;
        if (hthread == NULL)
            hthread = GetCurrentThread();

        std::string str;
        if (!SymInitialize(hprocess, NULL, TRUE)) 
        {
            str = ("Initialize dbghelp library ERROR!\n");
        }
        else
        {
            //UINT max_name_length;              // Max length of symbols' name.
            CONTEXT context;                   // Store register addresses.
            STACKFRAME64 stackframe;           // Call stack.
            PSYMBOL_INFO symbol;               // Debugging symbol's information.
            IMAGEHLP_LINE64 source_info;       // Source information (file name & line number)
            DWORD displacement;                // Source line displacement.
            std::ostringstream stack_info_str_stream;

            enum { MAX_NAME_LENGTH = 256 };  // max length of symbols' name.
            // Initialize PSYMBOL_INFO structure.  
            // Allocate a properly-sized block.  
            symbol = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (MAX_NAME_LENGTH - 1) * sizeof(TCHAR));
            memset(symbol, 0, sizeof(SYMBOL_INFO) + (MAX_NAME_LENGTH - 1) * sizeof(TCHAR));
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);  // SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).  
            symbol->MaxNameLen = MAX_NAME_LENGTH;
            // Initialize IMAGEHLP_LINE64 structure.  
            memset(&source_info, 0, sizeof(IMAGEHLP_LINE64));
            source_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            // Initialize STACKFRAME64 structure.  
            RtlCaptureContext(&context);  // Get context.  
            memset(&stackframe, 0, sizeof(STACKFRAME64));
            stackframe.AddrPC.Offset = context.Eip;  // Fill in register addresses (EIP, ESP, EBP).  
            stackframe.AddrPC.Mode = AddrModeFlat;
            stackframe.AddrStack.Offset = context.Esp;
            stackframe.AddrStack.Mode = AddrModeFlat;
            stackframe.AddrFrame.Offset = context.Ebp;
            stackframe.AddrFrame.Mode = AddrModeFlat;
            stack_info_str_stream.str("");

            int i = 0;
            stack_info_str_stream << "Call stack: \n";
            // Enumerate call stack frame.  
            while (StackWalk64(IMAGE_FILE_MACHINE_I386, hprocess, hthread, &stackframe,
                &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
                if (stackframe.AddrFrame.Offset == 0) {  // End reaches.  
                    break;
                }
                if (SymFromAddr(hprocess, stackframe.AddrPC.Offset, NULL, symbol)) {  // Get symbol.  
                    stack_info_str_stream << " ==> " << symbol->Name << "\n";
                }
                if (SymGetLineFromAddr64(hprocess, stackframe.AddrPC.Offset, &displacement, &source_info)) {
                    // Get source information.  
                    stack_info_str_stream << "\t[" << source_info.FileName << ":" << source_info.LineNumber << "]\n";
                }
                else {
                    if (GetLastError() == 0x1E7) {  // If err_code == 0x1e7, no symbol was found.  
                        stack_info_str_stream << "\tNo debug symbol loaded for this function.\n";
                    }
                }
            }

            stack_info_str_stream << "StackDumper is cleaned up!\n";
            str = stack_info_str_stream.str();

            free(symbol);
            // Clean up and exit. 
            SymCleanup(hprocess);
        }
        
        if (_hprocess == NULL)
            CloseHandle(hprocess);

        if (_hthread == NULL)
            CloseHandle(hthread);

        return str;
    }

    static bool DumpSimpleStack(std::list<DWORD64>& ll, HANDLE _hprocess = NULL, HANDLE _hthread = NULL)
    {
        HANDLE hprocess = _hprocess;
        if (hprocess == NULL)
            hprocess = GetCurrentProcess();

        HANDLE hthread = _hthread;
        if (hthread == NULL)
            hthread = GetCurrentThread();

        {
            //UINT max_name_length;              // Max length of symbols' name.
            CONTEXT context;                   // Store register addresses.
            STACKFRAME64 stackframe;           // Call stack.
            PSYMBOL_INFO symbol;               // Debugging symbol's information.
            IMAGEHLP_LINE64 source_info;       // Source information (file name & line number)

            enum { MAX_NAME_LENGTH = 256 };  // max length of symbols' name.
            // Initialize PSYMBOL_INFO structure.  
            // Allocate a properly-sized block.  
            symbol = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (MAX_NAME_LENGTH - 1) * sizeof(TCHAR));
            memset(symbol, 0, sizeof(SYMBOL_INFO) + (MAX_NAME_LENGTH - 1) * sizeof(TCHAR));
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);  // SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).  
            symbol->MaxNameLen = MAX_NAME_LENGTH;
            // Initialize IMAGEHLP_LINE64 structure.  
            memset(&source_info, 0, sizeof(IMAGEHLP_LINE64));
            source_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            // Initialize STACKFRAME64 structure.  
            RtlCaptureContext(&context);  // Get context.  
            memset(&stackframe, 0, sizeof(STACKFRAME64));
            stackframe.AddrPC.Offset = context.Eip;  // Fill in register addresses (EIP, ESP, EBP).  
            stackframe.AddrPC.Mode = AddrModeFlat;
            stackframe.AddrStack.Offset = context.Esp;
            stackframe.AddrStack.Mode = AddrModeFlat;
            stackframe.AddrFrame.Offset = context.Ebp;
            stackframe.AddrFrame.Mode = AddrModeFlat;
            //stack_info_str_stream.str("");

            int i = 0;
            //stack_info_str_stream << "Call stack: \n";
            // Enumerate call stack frame.  
            while (StackWalk64(IMAGE_FILE_MACHINE_I386, hprocess, hthread, &stackframe,
                &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
                if (stackframe.AddrFrame.Offset == 0) {  // End reaches.  
                    break;
                }
                ll.push_back(stackframe.AddrPC.Offset);
            }

            free(symbol);
        }

        if (_hprocess == NULL)
            CloseHandle(hprocess);

        if (_hthread == NULL)
            CloseHandle(hthread);

        return true;
    }

    static std::string ParseSimpleStack(const std::list<DWORD64>& ll, HANDLE _hprocess = NULL)
    {
        HANDLE hprocess = _hprocess;
        if (hprocess == NULL)
            hprocess = GetCurrentProcess();

        std::string str;
        if (!SymInitialize(hprocess, NULL, TRUE))
        {
            str = ("Initialize dbghelp library ERROR!\n");
        }
        else
        {
            //UINT max_name_length;              // Max length of symbols' name.
            CONTEXT context;                   // Store register addresses.
            STACKFRAME64 stackframe;           // Call stack.
            PSYMBOL_INFO symbol;               // Debugging symbol's information.
            IMAGEHLP_LINE64 source_info;       // Source information (file name & line number)
            DWORD displacement;                // Source line displacement.
            std::ostringstream stack_info_str_stream;

            enum { MAX_NAME_LENGTH = 256 };  // max length of symbols' name.
            // Initialize PSYMBOL_INFO structure.  
            // Allocate a properly-sized block.  
            symbol = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (MAX_NAME_LENGTH - 1) * sizeof(TCHAR));
            memset(symbol, 0, sizeof(SYMBOL_INFO) + (MAX_NAME_LENGTH - 1) * sizeof(TCHAR));
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);  // SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).  
            symbol->MaxNameLen = MAX_NAME_LENGTH;
            // Initialize IMAGEHLP_LINE64 structure.  
            memset(&source_info, 0, sizeof(IMAGEHLP_LINE64));
            source_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            
            // Initialize STACKFRAME64 structure.  
//             RtlCaptureContext(&context);  // Get context.  
//             memset(&stackframe, 0, sizeof(STACKFRAME64));
//             stackframe.AddrPC.Offset = context.Eip;  // Fill in register addresses (EIP, ESP, EBP).  
//             stackframe.AddrPC.Mode = AddrModeFlat;
//             stackframe.AddrStack.Offset = context.Esp;
//             stackframe.AddrStack.Mode = AddrModeFlat;
//             stackframe.AddrFrame.Offset = context.Ebp;
//             stackframe.AddrFrame.Mode = AddrModeFlat;
            stack_info_str_stream.str("");

            int i = 0;
            stack_info_str_stream << "Call stack: \n";
            // Enumerate call stack frame.  
            for (auto it = ll.begin(); it != ll.end(); it++) {
                
                DWORD64 ip = *it;
                if (SymFromAddr(hprocess, ip, NULL, symbol)) {  // Get symbol.  
                    stack_info_str_stream << " ==> " << symbol->Name << "\n";
                }
                if (SymGetLineFromAddr64(hprocess, ip, &displacement, &source_info)) {
                    // Get source information.  
                    stack_info_str_stream << "\t[" << source_info.FileName << ":" << source_info.LineNumber << "]\n";
                }
                else {
                    if (GetLastError() == 0x1E7) {  // If err_code == 0x1e7, no symbol was found.  
                        stack_info_str_stream << "\tNo debug symbol loaded for this function.\n";
                    }
                }
            }

            stack_info_str_stream << "StackDumper is cleaned up!\n";
            str = stack_info_str_stream.str();

            free(symbol);
            // Clean up and exit. 
            SymCleanup(hprocess);
        }

        if (_hprocess == NULL)
            CloseHandle(hprocess);

        return str;
    }
};

//////////////////////////////////////////////////////////////////////////
class MyStacks
{
public:
    void Add(const std::string& type, int key)
    {
        ScopedLock lock(cs_lock_);
        type_stacks_[type][key] = apihook::StackWalker::DumpFullStack();
    }

    void Add(const std::string& type, int key, const std::string& text)
    {
        ScopedLock lock(cs_lock_);
        type_stacks_[type][key] = text;
        //OutputDebugStringA("add\n");
    }

    void Remove(int key)
    {
        ScopedLock lock(cs_lock_);
        for (auto it = type_stacks_.begin(); it != type_stacks_.end(); it++)
        {
            auto it2 = it->second.find((int)key);
            if (it2 != it->second.end())
            {
                it->second.erase(it2);
                //OutputDebugStringA("remove\n");
                break;
            }
        }
    }

    void Clear()
    {
        ScopedLock lock(cs_lock_);
        type_stacks_.clear();
    }

    void Dump(const std::string& path)
    {
        ScopedLock lock(cs_lock_);
        std::ofstream file(path);

        int allcount = 0;
        for (auto it = type_stacks_.begin(); it != type_stacks_.end(); it++)
        {
            allcount += it->second.size();
        }

        file << "-----------------------------Begin----------------------------\n";
        file << "Found All: " << allcount << " leak\n";
        file << "--------------------------------------------------------------\n";
        for (auto it = type_stacks_.begin(); it != type_stacks_.end(); it++)
        {
            file << "Found " << it->first << ": " << it->second.size() << " leak\n";

            int index = 0;
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
            {
                file << it->first << " leak " << ++index << "==" << (int)it2->first << "\n";

                file << it2->second;

                file << "\n\n";
            }

        }
        file << "-----------------------------End----------------------------\n";

        file.close();
    }

protected:
    MyStacks()
    {
    }

    virtual ~MyStacks()
    {
    }

private:
    typedef std::unordered_map<int, std::string> STACKMAP;
    std::unordered_map<std::string, STACKMAP> type_stacks_;

    CSLock cs_lock_;
};

#define DEFINE_MYSTACK_INST(tag) \
    class MyStacks_##tag : public MyStacks{\
    public:\
        static MyStacks_##tag& Inst()\
        {\
            static MyStacks_##tag s;\
            return s;\
        }\
    }\

//////////////////////////////////////////////////////////////////////////
class MyStacksIPs
{
public:
    void Add(int key)
    {
        ScopedLock lock(cs_lock_);
        std::list<DWORD64> ll;
        apihook::StackWalker::DumpSimpleStack(ll);
        stacks_[key] = ll;

        ++all_add_;
    }

    void Remove(int key)
    {
        ScopedLock lock(cs_lock_);
        auto it2 = stacks_.find((int)key);
        if (it2 != stacks_.end())
        {
            stacks_.erase(it2);

            ++all_remove_;
        }
    }

    void Clear()
    {
        ScopedLock lock(cs_lock_);
        stacks_.clear();

        all_add_ = 0;
        all_remove_ = 0;
    }

    void Dump(const std::string& path)
    {
        ScopedLock lock(cs_lock_);
        std::ofstream file(path);

        HANDLE hprocess = GetCurrentProcess();

        int allcount = stacks_.size();

        file << "-----------------------------Begin----------------------------\n";
        file << "Found All: " << allcount << " leak\n";
        file << "All add: " << all_add_ << " all remove: " << all_remove_ << "\n";
        file << "--------------------------------------------------------------\n";
        for (auto it = stacks_.begin(); it != stacks_.end(); it++)
        {
            std::string str = apihook::StackWalker::ParseSimpleStack(it->second, hprocess);

            file << "addr: " << it->first << "\n";
            file << "callstacks:\n";
            file << str << "\n";
        }
        file << "-----------------------------End----------------------------\n";

        file.close();

        CloseHandle(hprocess);
    }

protected:
    MyStacksIPs()
    {
        all_add_ = 0;
        all_remove_ = 0;
    }

    virtual ~MyStacksIPs()
    {
    }

private:
    typedef std::list<DWORD64> STACKIPLIST;
    std::unordered_map<int, STACKIPLIST> stacks_;

    int all_add_;
    int all_remove_;

    CSLock cs_lock_;
};

#define DEFINE_MYSTACKIPS_INST(tag) \
    class MyStacksIPs_##tag : public MyStacksIPs{\
    public:\
        static MyStacksIPs_##tag& Inst()\
        {\
            static MyStacksIPs_##tag s;\
            return s;\
        }\
    }\

}