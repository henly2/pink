// Hooker.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <windows.h>

#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

#include <iostream>
using namespace std;

#include "../Hooker/common.h"
using namespace hook;

#include "..\HookerDLL\HookInjEx_DLL.h"
#pragma comment(lib, "../debug/HookerDll.lib")

#include "../source/apihook/APIHook2.hpp"
#include "../source/apihook/memory/MemoryHook.hpp"

//////////////////////////////////////////////////////////////////////////
DWORD FindProcByName(LPCSTR lpName)
{
    DWORD aProcId[1024], dwProcCnt, dwModCnt;
    char szPath[MAX_PATH];
    HMODULE hMod;

    //枚举出所有进程ID
    if (!EnumProcesses(aProcId, sizeof(aProcId), &dwProcCnt))
    {
        //cout << "EnumProcesses error: " << GetLastError() << endl;
        return 0;
    }

    //遍例所有进程
    for (DWORD i = 0; i < dwProcCnt; ++i)
    {
        //打开进程，如果没有权限打开则跳过
        HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, aProcId[i]);
        if (NULL != hProc)
        {
            //打开进程的第1个Module，并检查其名称是否与目标相符
            if (EnumProcessModules(hProc, &hMod, sizeof(hMod), &dwModCnt))
            {
                GetModuleBaseNameA(hProc, hMod, szPath, MAX_PATH);
                if (0 == _stricmp(szPath, lpName))
                {
                    CloseHandle(hProc);
                    return aProcId[i];
                }
            }
            CloseHandle(hProc);
        }
    }
    return 0;
}

typedef struct
{
    HWND hWnd;
    DWORD dwPid;
}WNDINFO;

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    WNDINFO* pInfo = (WNDINFO*)lParam;
    DWORD dwProcessId = 0;
    GetWindowThreadProcessId(hWnd, &dwProcessId);

    if (dwProcessId == pInfo->dwPid)
    {
        pInfo->hWnd = hWnd;
        return FALSE;
    }
    return TRUE;
}

HWND GetHwndByProcessId(DWORD dwProcessId)
{
    WNDINFO info = { 0 };
    info.hWnd = NULL;
    info.dwPid = dwProcessId;
    EnumWindows(EnumWindowsProc, (LPARAM)&info);
    return info.hWnd;
}

//////////////////////////////////////////////////////////////////////////
void printusage()
{
    std::cout << "Usage:\n";
    std::cout << "  0: " << "listprocess (list all process)\n";
    std::cout << "  1: " << "attach pid/name (inject a process)\n";
    std::cout << "  2: " << "detach (detach process)\n";
    std::cout << "  3: " << "quit (detach process and quit)\n";
    std::cout << "  4: " << "he " << g_all_gdiitems << " (hook enable item)\n";
    std::cout << "  5: " << "hd " << g_all_gdiitems << " (hook disable item)\n";
    std::cout << "  6: " << "clear (clear leak)\n";
    std::cout << "  7: " << "dump (dump leak)\n";
}

void errinput()
{
    std::cout << "###Err: input...\n";
    printusage();
}

//////////////////////////////////////////////////////////////////////////
HANDLE g_htarget = NULL;
DWORD g_dll = 0;

HWND FindTargetHwnd()
{
    HWND hwnd = NULL;

    hwnd = FindWindowA(CLASS_NAME_DLL, NULL);
    if (hwnd == NULL)
    {
        hwnd = FindWindowA(NULL, CLASS_NAME_DLL);
    }

    return hwnd;
}

HWND FindHostHwnd()
{
    HWND hwnd = NULL;

    hwnd = FindWindowA(CLASS_NAME_HOST, NULL);
    if (hwnd == NULL)
    {
        hwnd = FindWindowA(NULL, CLASS_NAME_HOST);
    }

    return hwnd;
}

void listprocess()
{
    DWORD aProcId[1024], dwProcCnt, dwModCnt;
    char szPath[MAX_PATH];
    HMODULE hMod;

    //枚举出所有进程ID
    if (!EnumProcesses(aProcId, sizeof(aProcId), &dwProcCnt))
    {
        std::cout << "EnumProcesses error: " << GetLastError() << std::endl;
    }

    //遍例所有进程
    for (DWORD i = 0; i < dwProcCnt; ++i)
    {
        //打开进程，如果没有权限打开则跳过
        HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, aProcId[i]);
        if (NULL != hProc)
        {
            //打开进程的第1个Module，并检查其名称是否与目标相符
            if (EnumProcessModules(hProc, &hMod, sizeof(hMod), &dwModCnt))
            {
                GetModuleBaseNameA(hProc, hMod, szPath, MAX_PATH);

                std::cout << szPath << "    " << aProcId[i] << std::endl;
                
            }
            CloseHandle(hProc);
        }
    }
}
void attach(std::vector<std::string>& vec)
{
    std::cout << "attach..." << std::endl;
    if (g_htarget != NULL)
    {
        std::cout << "###Err:has attach" << std::endl;
        return;
    }

    if (vec.size() < 2){
        errinput();
        return;
    }

    int pid = atoi(vec[1].c_str());
    if (pid == 0){
        pid = FindProcByName(vec[1].c_str());
    }
    if (pid == 0){
        std::cout << "###Err: process id or name...\n";
        return;
    }

    HANDLE hTarget = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (NULL == hTarget)
    {
        std::cout << "###Err: Can't Open target process!\n";
        return;
    }

    HWND hStart = GetHwndByProcessId(pid);
    InjectDll(hStart);
    
    //++
    g_htarget = hTarget;

    apihook::StackWalkerIPC::Inst().EnableRemote(pid);

    std::cout << "attach finish..., module = " << g_dll << std::endl;
}
void detach(std::vector<std::string>& vec)
{
    std::cout << "detach..." << std::endl;

    if (g_htarget == NULL)
    {
        std::cout << "###Err:no attach" << std::endl;
        return;
    }

    apihook::StackWalkerIPC::Inst().DisableRemote();

    UnmapDll();

    CloseHandle(g_htarget);
    g_htarget = NULL;
    g_dll = 0;

    std::cout << "detach finish..." << std::endl;
    return;
}
void he(std::vector<std::string>& vec)
{
    if (vec.size() < 2){
        errinput();
        return;
    }

    int value = compose_gdiids(vec[1]);
    HWND ht = FindTargetHwnd();
    if (ht == NULL)
    {
        std::cout << "###Err: not find hwnd\n";
        return;
    }
    
    PostMessage(ht, WM_IPC_TODLL, DEF_FUNC_ITEM_ID(he), value);
}
void hd(std::vector<std::string>& vec)
{
    if (vec.size() < 2){
        errinput();
        return;
    }

    int value = compose_gdiids(vec[1]);
    HWND ht = FindTargetHwnd();
    if (ht == NULL)
    {
        std::cout << "###Err: not find hwnd\n";
        return;
    }

    PostMessage(ht, WM_IPC_TODLL, DEF_FUNC_ITEM_ID(hd), value);
}
void clear()
{
    system("cls");

    {
        HWND ht = FindTargetHwnd();
        if (ht == NULL)
        {
            std::cout << "###Err: not find hwnd\n";
            return;
        }

        PostMessage(ht, WM_IPC_TODLL, DEF_FUNC_ITEM_ID(clear), 0);
    }

    {
        HWND ht = FindHostHwnd();
        if (ht == NULL)
        {
            std::cout << "###Err: not find host hwnd\n";
            return;
        }

        PostMessage(ht, WM_IPC_TOHOST, DEF_FUNC_ITEM_ID(clear), 0);
    }
}
void dump()
{
    std::cout << "dump..." << std::endl;

    {
        HWND ht = FindTargetHwnd();
        if (ht == NULL)
        {
            std::cout << "###Err: not find hwnd\n";
            return;
        }

        PostMessage(ht, WM_IPC_TODLL, DEF_FUNC_ITEM_ID(dump), 0);
    }

    {
        HWND ht = FindHostHwnd();
        if (ht == NULL)
        {
            std::cout << "###Err: not find host hwnd\n";
            return;
        }

        PostMessage(ht, WM_IPC_TOHOST, DEF_FUNC_ITEM_ID(dump), 0);
    }
    
}

void me(std::vector<std::string>& vec)
{
    if (vec.size() < 2){
        errinput();
        return;
    }

    int value = atoi(vec[1].c_str());
    HWND ht = FindTargetHwnd();
    if (ht == NULL)
    {
        std::cout << "###Err: not find hwnd\n";
        return;
    }

    PostMessage(ht, WM_IPC_TODLL, DEF_FUNC_ITEM_ID(me), value);
}
void md(std::vector<std::string>& vec)
{
    if (vec.size() < 2){
        errinput();
        return;
    }

    int value = atoi(vec[1].c_str());
    HWND ht = FindTargetHwnd();
    if (ht == NULL)
    {
        std::cout << "###Err: not find hwnd\n";
        return;
    }

    PostMessage(ht, WM_IPC_TODLL, DEF_FUNC_ITEM_ID(md), value);
}
//////////////////////////////////////////////////////////////////////////
bool doonce(const std::string& input)
{
    std::vector<std::string> vec;
    spliter_by_char(input, ' ', vec);

    if (vec.size() < 1){
        errinput();
        return false;
    }

    bool bquit = false;
    FUNCID fid = getfuncid_bystring(vec[0]);
    switch (fid)
    {
    case Func_nil:
        errinput();
        break;
    case Func_listprocess:
        listprocess();
        break;
    case Func_attach:
        attach(vec);
        break;
    case Func_detach:
        detach(vec);
        break;
    case Func_quit:
        detach(vec);
        bquit = true;
        break;
    case Func_he:
        he(vec);
        break;
    case Func_hd:
        hd(vec);
        break;
    case Func_clear:
        clear();
        break;
    case Func_dump:
        dump();
        break;
    case Func_me:
        me(vec);
        break;
    case Func_md:
        md(vec);
        break;
    default:
        break;
    }

    return bquit;
}

//////////////////////////////////////////////////////////////////////////
HANDLE g_hconsole = INVALID_HANDLE_VALUE;
bool initconsole(){
    if (g_hconsole != INVALID_HANDLE_VALUE)
    {
        return true;
    }

    if (!AllocConsole())
    {
        return false;
    }

    g_hconsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (g_hconsole == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    SetConsoleTitleA("Debugger");
    return true;
}

void uninitconsole()
{
    g_hconsole = NULL;
    FreeConsole();
}

//////////////////////////////////////////////////////////////////////////
int inithooker()
{
    //initconsole();

    init_funcid();
    init_gdiid();

    const char* version = "1.0";
    std::cout << "Hooker version " << version << "\n";
    printusage();
    std::cout << "=================================================\n";

    std::cout << "Support GDIs:\n";
    std::cout << g_all_gdiitems << "\n";
    
	return 0;
}

void uninithooker()
{
    //uninitconsole();
}

HANDLE gettarget()
{
    return g_htarget;
}

void docommand(const std::string& input)
{
    doonce(input);
}