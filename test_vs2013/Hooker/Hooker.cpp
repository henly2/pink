// Hooker.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <windows.h>

#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

#include <iostream>

using namespace std;

#include "common.h"
using namespace hook;

#include "..\HookerDLL\HookInjEx_DLL.h"
#pragma comment(lib, "../debug/HookerDll.lib")

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

    return hwnd;
}

HWND FindHostHwnd()
{
    HWND hwnd = NULL;

    hwnd = FindWindowA(CLASS_NAME_HOST, NULL);

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

    //获取LoadLibraryW和FreeLibrary在宿主进程中的入口点地址
    HMODULE hKernel32 = GetModuleHandle(_T("Kernel32"));
    LPTHREAD_START_ROUTINE pLoadLib = (LPTHREAD_START_ROUTINE)
        GetProcAddress(hKernel32, "LoadLibraryA");
    if (NULL == pLoadLib)
    {
        std::cout << "Library procedure not found: " << GetLastError() << std::endl;
        CloseHandle(hTarget);
        return;
    }

    std::string hookerdll = GetModuleDir(GetModuleHandleA(NULL));
    hookerdll += "\\";
    hookerdll += "hookerdll.dll";
    CHAR szPath[MAX_PATH] = {0};
    strcpy_s(szPath, hookerdll.c_str());
    //在宿主进程中为LoadLibraryW的参数分配空间，并将参数值写入
    LPVOID lpMem = VirtualAllocEx(hTarget, NULL, sizeof(szPath),
        MEM_COMMIT, PAGE_READWRITE);
    if (NULL == lpMem)
    {
        std::cout << "Can't alloc memory block: " << GetLastError() << std::endl;
        CloseHandle(hTarget);
        return;
    }

    // 参数即为要注入的DLL的文件路径
    if (!WriteProcessMemory(hTarget, lpMem, (void*)szPath, sizeof(szPath), NULL))
    {
        std::cout << "Can't write parameter to memory: " << GetLastError() << std::endl;
        VirtualFreeEx(hTarget, lpMem, sizeof(szPath), MEM_RELEASE);
        CloseHandle(hTarget);
        return;
    }

    //创建信号量，DLL代码可以通过ReleaseSemaphore来通知主程序清理
    //==HANDLE hSema = CreateSemaphore(NULL, 0, 1, _T("Global\\InjHack"));

    //将DLL注入宿主进程
    HANDLE hThread = CreateRemoteThread(hTarget, NULL, 0, pLoadLib, lpMem, 0, NULL);

    //释放宿主进程内的参数内存
    VirtualFreeEx(hTarget, lpMem, sizeof(szPath), MEM_RELEASE);

    if (NULL == hThread)
    {
        std::cout << "Can't create remote thread: " << GetLastError() << std::endl;
        CloseHandle(hTarget);
        return;
    }

    //等待DLL信号量或宿主进程退出
    WaitForSingleObject(hThread, INFINITE);
    /*==
    HANDLE hObj[2] = { hTarget, hSema };
    if (WAIT_OBJECT_0 == WaitForMultipleObjects(2, hObj, FALSE, INFINITE))
    {
        cout << "Target process exit." << endl;
        CloseHandle(hTarget);
        return;
    }
    CloseHandle(hSema);

    //根据线程退出代码获取DLL的Module ID
    DWORD dwLibMod;
    if (!GetExitCodeThread(hThread, &dwLibMod))
    {
        cout << "Can't get return code of LoadLibrary: " << GetLastError() << endl;
        CloseHandle(hThread);
        CloseHandle(hTarget);
        return;
    }==*/

    //++
    if (!GetExitCodeThread(hThread, &g_dll))
    {
        cout << "Can't get return code of LoadLibrary: " << GetLastError() << endl;
        CloseHandle(hThread);
        CloseHandle(hTarget);
        return;
    }

    //关闭线程句柄
    CloseHandle(hThread);
    //++
    g_htarget = hTarget;

    std::cout << "attach finish..., module = " << g_dll << std::endl;
}
void attach2(std::vector<std::string>& vec)
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

    HMODULE hKernel32 = GetModuleHandle(_T("Kernel32"));
    LPTHREAD_START_ROUTINE pFreeLib = (LPTHREAD_START_ROUTINE)
        GetProcAddress(hKernel32, "FreeLibrary");
    if (NULL == pFreeLib)
    {
        std::cout << "Library procedure not found: " << GetLastError() << std::endl;
        CloseHandle(g_htarget);
        g_htarget = NULL;
        return;
    }

    //再次注入FreeLibrary代码以释放宿主进程加载的注入体DLL
    HANDLE hThread = CreateRemoteThread(g_htarget, NULL, 0, pFreeLib, (void*)g_dll, 0, NULL);
    if (NULL == hThread)
    {
        cout << "Can't call FreeLibrary: " << GetLastError() << endl;
        CloseHandle(g_htarget);
        g_htarget = NULL;
        return;
    }
    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);

    CloseHandle(g_htarget);
    g_htarget = NULL;
    g_dll = 0;

    std::cout << "detach finish..." << std::endl;
    return;
}
void detach2(std::vector<std::string>& vec)
{
    std::cout << "detach..." << std::endl;

    if (g_htarget == NULL)
    {
        std::cout << "###Err:no attach" << std::endl;
        return;
    }

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

    int value = 0;// compose_gdiids(vec[1]);
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

    int value = 0;// compose_gdiids(vec[1]);
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
        attach2(vec);
        break;
    case Func_detach:
        detach2(vec);
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
int _tmain(int argc, _TCHAR* argv[])
{
    init_funcid();
    init_gdiid();

    const char* version = "1.0";
    std::cout << "Hooker version " << version << "\n";
    printusage();
    std::cout << "=================================================\n";

    std::cout << "Support GDIs:\n";
    std::cout << g_all_gdiitems << "\n";

    bool bquit = false;
    std::string input;
    while (!bquit){
        input.clear();
        std::cout << ">";
        std::getline(std::cin, input);

        bquit = doonce(input);
    }

	return 0;
}

