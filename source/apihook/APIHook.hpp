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
    static StackWalker& Inst()
    {
        static StackWalker s;
        return s;
    }

	bool Enable()
	{
        if (hprocess_ != NULL)
            return true;

        hprocess_ = GetCurrentProcess();

        // Initialize dbghelp library.  
        if (!SymInitialize(hprocess_, NULL, TRUE)) {
            OutputDebugStringA("Initialize dbghelp library ERROR!\n");

            CloseHandle(hprocess_);
            hprocess_ = NULL;
            return false;
        }

        return true;
	}

    void Disable()
    {
        if (hprocess_ == NULL)
            return;

        // Clean up and exit. 
        SymCleanup(hprocess_);

        CloseHandle(hprocess_);
        hprocess_ = NULL;
    }

public:
    std::string DumpStack()
    {
        if (hprocess_ == NULL)
            return "process is null";

        //UINT max_name_length;              // Max length of symbols' name.
        CONTEXT context;                   // Store register addresses.
        STACKFRAME64 stackframe;           // Call stack.
        HANDLE thread;           // Handle to current process & thread.
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

        thread = GetCurrentThread();

        int i = 0;
        stack_info_str_stream << "Call stack: \n";
        // Enumerate call stack frame.  
        while (StackWalk64(IMAGE_FILE_MACHINE_I386, hprocess_, thread, &stackframe,
            &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
            if (stackframe.AddrFrame.Offset == 0) {  // End reaches.  
                break;
            }
            if (SymFromAddr(hprocess_, stackframe.AddrPC.Offset, NULL, symbol)) {  // Get symbol.  
                stack_info_str_stream << " ==> " << symbol->Name << "\n";
            }
            if (SymGetLineFromAddr64(hprocess_, stackframe.AddrPC.Offset, &displacement, &source_info)) {
                // Get source information.  
                stack_info_str_stream << "\t[" << source_info.FileName << ":" << source_info.LineNumber << "]\n";
            }
            else {
                if (GetLastError() == 0x1E7) {  // If err_code == 0x1e7, no symbol was found.  
                    stack_info_str_stream << "\tNo debug symbol loaded for this function.\n";
                }
            }
        }

        free(symbol);
        CloseHandle(thread);

        stack_info_str_stream << "StackDumper is cleaned up!\n";

        return stack_info_str_stream.str();
    }

private:
    StackWalker(void)
        :hprocess_(NULL)
    {

    }

    ~StackWalker(void)
    {

    }

private:
    HANDLE hprocess_;
};

//////////////////////////////////////////////////////////////////////////
class MyStacks
{
public:
    void Add(const std::string& type, int key)
    {
        type_stacks_[type][key] = apihook::StackWalker::Inst().DumpStack();
    }

    void Remove(int key)
    {
        for (auto it = type_stacks_.begin(); it != type_stacks_.end(); it++)
        {
            auto it2 = it->second.find((int)key);
            if (it2 != it->second.end())
            {
                it->second.erase(it2);
                break;
            }
        }
    }

    void Clear()
    {
        type_stacks_.clear();
    }

    void Dump(const std::string& path)
    {
        std::ofstream file(path);

        int allcount = 0;
        for (auto it = type_stacks_.begin(); it != type_stacks_.end(); it++)
        {
            allcount += it->second.size();
        }

        file << "-----------------------------Begin----------------------------\n";
        file << "Found All GDI: " << allcount << " leak\n";
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

}