#pragma once

#include <sstream>

#include <Dbghelp.h>  
#include <DelayImp.h>  
#pragma comment( lib, "Dbghelp.lib")  
#define DBGHELP_TRANSLATE_TCHAR

namespace api_hook {

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

	void UnHook()
	{
		CallBefore();
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
class StackWalker {
public:
	StackWalker()
	{
	}
    
	~StackWalker()
	{
	}

	static std::string DumpStack()
	{
		if (s_hprocess == NULL)
			return "";

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
		
		stack_info_str_stream << "Call stack: \n";
		// Enumerate call stack frame.  
		while (StackWalk64(IMAGE_FILE_MACHINE_I386, s_hprocess, thread, &stackframe,
			&context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
			if (stackframe.AddrFrame.Offset == 0) {  // End reaches.  
				break;
			}
			if (SymFromAddr(s_hprocess, stackframe.AddrPC.Offset, NULL, symbol)) {  // Get symbol.  
				stack_info_str_stream << " ==> " << symbol->Name << "\n";
			}
			if (SymGetLineFromAddr64(s_hprocess, stackframe.AddrPC.Offset, &displacement, &source_info)) {
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

    static bool Enable()
    {
        if (s_hprocess != NULL)
            return true;

		s_hprocess = GetCurrentProcess();

        // Initialize dbghelp library.  
        if (!SymInitialize(s_hprocess, NULL, TRUE)) {
            OutputDebugStringA("Initialize dbghelp library ERROR!\n");
			
			CloseHandle(s_hprocess);
			s_hprocess = NULL;
			return false;
        }

		return true;
    }

    static void Disable()
    {
        if (s_hprocess == NULL)
            return;

		// Clean up and exit. 
        SymCleanup(s_hprocess);

        CloseHandle(s_hprocess);
		s_hprocess = NULL;
    }
    
private:
	static HANDLE s_hprocess;
};
HANDLE StackWalker::s_hprocess = NULL;

//////////////////////////////////////////////////////////////////////////
class CAPIHook
{
public:
	CAPIHook(void);
	~CAPIHook(void);

	static bool Init()
	{
		StackWalker::Enable();

		return true;
	}

	static void UnInit()
	{
		StackWalker::Disable();
	}
};

}