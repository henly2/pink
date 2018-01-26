#include "stdafx.h"

#include <windows.h>

#include "../include/log/console_log.hpp"
/*
�߳��˳���
1. endthread���������˳��̣߳����ܻ������Դй©������
2. terminatethread, ͬ�ϣ������أ�����ɳ����ȶ�����Ҫ��
3. ���ʹ��return����ֹ�߳�
4. closehandle(�߳̾��)��ֻ���ͷ��߳̾�����󣬲�û����ֹ�߳�
5. �����˳�ʱ������ȴ�һ����ʱ�߳�һֱû���˳�������ɳ������鲻�ã��ǾͲ�Ҫ�ȴ��ˣ��ó������߳��˳����Զ��˳��̣߳�
����������̵߳�ʱ����Ҫע�⣺
1 ʹ�þ�̬������Ϊ�̵߳Ĳ�����
2.������ƽӿڻص�������ʹ�ã�
3.ͬʱ�̱߳����쳣�˳��������Ӱ�죬����û��дһ����Ҫ���ļ���Ϊ�´��������ļ�����������Ҫ�ȴ��������´���������������
4.��������߳��ڲ�ÿ��ִ�е�ʱ��С���̣������http������Ӧ���ǿ���ֱ���˳�

��������ӿ��ܻ���ɱ����ġ���
���Ը���ɾ�̬��������������꽫�������ø��ⲿ���󣬵��ⲿ����ΪNULLʱ���򲻲���
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
