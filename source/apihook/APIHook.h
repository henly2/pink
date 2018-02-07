#pragma once

#include <sstream>

#include <Dbghelp.h>  
#include <DelayImp.h>  
#pragma comment( lib, "Dbghelp.lib")  

//////////////////////////////////////////////////////////////////////////
template<typename FUNC>
class APIHookInfo
{
    FUNC yourfunc;

    unsigned char code[5];
    unsigned char oldcode[5];

public:
    FUNC hookfunc;

    APIHookInfo(){
        memset(this, 0, sizeof(*this));
    }

    void Hook(const char* dllname, const char* funcname, FUNC yourfunc){
        HMODULE hdll = GetModuleHandleA(dllname);
        hookfunc = (FUNC)GetProcAddress(hdll, funcname);

        if (hookfunc){
            code[0] = 0xe9;
            DWORD a = (DWORD)yourfunc - (DWORD)hookfunc - 5;
            RtlMoveMemory(code + 1, &a, 4);

            DWORD old;
            if (VirtualProtectEx(GetCurrentProcess(), hookfunc, 5, PAGE_EXECUTE_READWRITE, &old)){
                RtlMoveMemory(oldcode, hookfunc, 5);
                WriteProcessMemory(GetCurrentProcess(), hookfunc, code, 5, NULL);
                VirtualProtectEx(GetCurrentProcess(), hookfunc, 5, old, &old);
            }
        }
    }

    void CallBefore()
    {
        DWORD old;
        if (VirtualProtectEx(GetCurrentProcess(), hookfunc, 5, PAGE_EXECUTE_READWRITE, &old)){
            WriteProcessMemory(GetCurrentProcess(), hookfunc, oldcode, 5, NULL);
            VirtualProtectEx(GetCurrentProcess(), hookfunc, 5, old, &old);
        }
    }

    void CallAfter()
    {
        DWORD old;
        if (VirtualProtectEx(GetCurrentProcess(), hookfunc, 5, PAGE_EXECUTE_READWRITE, &old)){
            WriteProcessMemory(GetCurrentProcess(), hookfunc, code, 5, NULL);
            VirtualProtectEx(GetCurrentProcess(), hookfunc, 5, old, &old);
        }
    }
};

//////////////////////////////////////////////////////////////////////////
#define DBGHELP_TRANSLATE_TCHAR
class StackDumper {
public:
    StackDumper();
    ~StackDumper();
    void Destory();
    std::string DumpStack();

    std::string out;

    static void init()
    {
        if (process != NULL)
            return;

        process = GetCurrentProcess();
        // Initialize dbghelp library.  
        if (!SymInitialize(process, NULL, TRUE)) {
            OutputDebugStringA("Initialize dbghelp library ERROR!\n");
        }
    }
    static void uninit()
    {
        if (process == NULL)
            return;

        SymCleanup(process);  // Clean up and exit.  
        CloseHandle(process);
    }
    static HANDLE process;

private:
    UINT max_name_length_;              // Max length of symbols' name.
    CONTEXT context_;                   // Store register addresses.
    STACKFRAME64 stackframe_;           // Call stack.
    HANDLE thread_;           // Handle to current process & thread.
    PSYMBOL_INFO symbol_;               // Debugging symbol's information.
    IMAGEHLP_LINE64 source_info_;       // Source information (file name & line number)
    DWORD displacement_;                // Source line displacement.
    std::ostringstream stack_info_str_stream_;
};

HANDLE StackDumper::process = NULL;
StackDumper::StackDumper() {

    enum { MAX_NAME_LENGTH = 256 };  // max length of symbols' name.
    // Initialize PSYMBOL_INFO structure.  
    // Allocate a properly-sized block.  
    symbol_ = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (MAX_NAME_LENGTH - 1) * sizeof(TCHAR));
    memset(symbol_, 0, sizeof(SYMBOL_INFO) + (MAX_NAME_LENGTH - 1) * sizeof(TCHAR));
    symbol_->SizeOfStruct = sizeof(SYMBOL_INFO);  // SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).  
    symbol_->MaxNameLen = MAX_NAME_LENGTH;
    // Initialize IMAGEHLP_LINE64 structure.  
    memset(&source_info_, 0, sizeof(IMAGEHLP_LINE64));
    source_info_.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    // Initialize STACKFRAME64 structure.  
    RtlCaptureContext(&context_);  // Get context.  
    memset(&stackframe_, 0, sizeof(STACKFRAME64));
    stackframe_.AddrPC.Offset = context_.Eip;  // Fill in register addresses (EIP, ESP, EBP).  
    stackframe_.AddrPC.Mode = AddrModeFlat;
    stackframe_.AddrStack.Offset = context_.Esp;
    stackframe_.AddrStack.Mode = AddrModeFlat;
    stackframe_.AddrFrame.Offset = context_.Ebp;
    stackframe_.AddrFrame.Mode = AddrModeFlat;
    stack_info_str_stream_.str("");

    thread_ = GetCurrentThread();
    out = DumpStack();
}

StackDumper::~StackDumper() {
    Destory();
}

void StackDumper::Destory() {
    free(symbol_);
    stack_info_str_stream_ << "StackDumper is cleaned up!\n";
}

std::string StackDumper::DumpStack() {

    stack_info_str_stream_ << "Call stack: \n";
    // Enumerate call stack frame.  
    while (StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread_, &stackframe_,
        &context_, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
        if (stackframe_.AddrFrame.Offset == 0) {  // End reaches.  
            break;
        }
        if (SymFromAddr(process, stackframe_.AddrPC.Offset, NULL, symbol_)) {  // Get symbol.  
            stack_info_str_stream_ << " ==> " << symbol_->Name << "\n";
        }
        if (SymGetLineFromAddr64(process, stackframe_.AddrPC.Offset, &displacement_, &source_info_)) {
            // Get source information.  
            stack_info_str_stream_ << "\t[" << source_info_.FileName << ":" << source_info_.LineNumber << "]\n";
        }
        else {
            if (GetLastError() == 0x1E7) {  // If err_code == 0x1e7, no symbol was found.  
                stack_info_str_stream_ << "\tNo debug symbol loaded for this function.\n";
            }
        }
    }

    return stack_info_str_stream_.str();
}

//////////////////////////////////////////////////////////////////////////
class CAPIHook
{
public:
    
public:
	CAPIHook(void);
	~CAPIHook(void);

    static bool Start();
    static void Stop();

    static void Reset();
    static void Dump();

    static std::unordered_map<INT_PTR, StackDumper*> g_stacks;
};