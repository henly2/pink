#include "stdafx.h"

#include <windows.h>

#include "../include/log/console_log.hpp"
/*
线程退出：
1. endthread，会立马退出线程，可能会造成资源泄漏，慎用
2. terminatethread, 同上，更严重，会造成程序不稳定，不要用
3. 最好使用return来终止线程
4. closehandle(线程句柄)，只是释放线程句柄对象，并没有终止线程
5. 程序退出时，如果等待一个耗时线程一直没有退出，会造成程序体验不好，那就不要等待了，让程序主线程退出来自动退出线程，
所以在设计线程的时候，需要注意：
1 使用静态对象作为线程的参数，
2.可以设计接口回调给外面使用，
3.同时线程本身异常退出不会造成影响，比如没有写一个重要的文件作为下次启动的文件，这样必须要等待，而且下次启动最好能做检测
4.尽量设计线程内部每次执行的时间小而短，如果是http请求，则应该是可以直接退出

下面的例子可能会造成崩溃的。。
可以改造成静态对象操作，操作完将数据设置给外部对象，当外部对象为NULL时，则不操作
*/

static HANDLE g_thread = NULL;
DWORD WINAPI ThreadFunc(LPVOID lpParam)
{
    int* value = (int*)lpParam;
    while (true)
    {
        (*value)++;

        Sleep(5000);
        
        util::Console_log::ins().postLog("value == %d", *value);
    }
    
    return 0;
}

int* g_int = nullptr;
void test_start_thread()
{
    util::Console_log::ins().postLog("test_start_thread begin...");
    
    g_int = new int(0);
    g_thread = CreateThread(NULL, 0, ThreadFunc, (LPVOID)(g_int), 0, 0);

    util::Console_log::ins().postLog("test_start_thread end...");
}
void test_stop_thread()
{
    util::Console_log::ins().postLog("test_stop_thread begin...");

    if (g_thread != NULL)
    {
        WaitForSingleObject(g_thread, 2000);
        CloseHandle(g_thread);
        g_thread = NULL;
    }

    if (g_int != nullptr)
    {
        delete g_int;
        g_int = nullptr;
    }

    util::Console_log::ins().postLog("test_stop_thread end...");
}
