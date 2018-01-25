#include "stdafx.h"

#include <windows.h>

#include <time.h>
#include <iostream>

#include "../include/text/dbc_sbc.h"

std::string FromWideString(const wchar_t *pwstr)
{
    std::string out;

    int len = pwstr ? wcslen(pwstr) : 0;
    if (len > 0)
    {
        int dwNum = WideCharToMultiByte(936/*CP_OEMCP*/, NULL, pwstr, len, NULL, 0, NULL, FALSE);
        if (dwNum > 0)
        {
            char* buf = new char[dwNum + 1];
            WideCharToMultiByte(936/*CP_OEMCP*/, NULL, pwstr, len, buf, dwNum+1, NULL, FALSE);
            out.append(buf, dwNum);
        }
    }

    return out;
}

#define PINK_EQUAL(x, y) \
    {\
    std::string msg;\
    msg += #x;\
    msg += " != ";\
    msg += #y;\
    std::cout << #x << "==" << x << std::endl;\
    std::cout << #y << "==" << y << std::endl;\
    if(x != y)\
        {\
        std::cout << #x << "==" << #y << "[failed]" << std::endl;\
        }else \
    {\
        std::cout << #x << "==" << #y << "[ok]" << std::endl;\
    }\
    std::cout << std::endl;\
    }\

// log
std::ostream& pinkBeginLog(const char* tag)
{ 
    time_t nowtime = time(nullptr);
    struct tm nowtm;
    localtime_s(&nowtm, &nowtime);

    char buf[1024] = { 0 };
    _snprintf_s(buf, 1024, _TRUNCATE, "[%04d%02d%02d %02d%02d%02d][%s][%d]", 
        nowtm.tm_year + 1900,
        nowtm.tm_mon + 1,
        nowtm.tm_mday,
        nowtm.tm_hour,
        nowtm.tm_min,
        nowtm.tm_sec,
        tag,
        GetCurrentThreadId());

    std::cout << buf;
    return std::cout; 
}

#define PINKLOG_DEBUG pinkBeginLog("debug")<< "[" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << "]"
#define PINKLOG_INFO pinkBeginLog("info")<< "[" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << "]"
#define PINKLOG_WARNING pinkBeginLog("warning")<< "[" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << "]"
#define PINKLOG_ERROR pinkBeginLog("error")<< "[" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << "]"

void test_text()
{
    std::wstring dbc_str = L"a~`!@#$%^&*()_-+= 1我们";
    std::wstring sbc_str = L"ａ～｀！＠＃＄％＾＆＊（）＿－＋＝　１我们";
    
    // d-s
    std::wstring sbc_out;
    text::dbc_2_sbc(dbc_str, sbc_out);
    PINK_EQUAL(FromWideString(sbc_str.c_str()), FromWideString(sbc_out.c_str()));

    text::dbc_2_sbc(sbc_str, sbc_out);
    PINK_EQUAL(FromWideString(sbc_str.c_str()), FromWideString(sbc_out.c_str()));

    // s-d
    std::wstring dbc_out;
    text::sbc_2_dbc(sbc_str, dbc_out);
    PINK_EQUAL(FromWideString(dbc_str.c_str()), FromWideString(dbc_out.c_str()));

    text::sbc_2_dbc(dbc_str, dbc_out);
    PINK_EQUAL(FromWideString(dbc_str.c_str()), FromWideString(dbc_out.c_str()));
}

//////////////////////////////////////////////////////////////////////////
void test_text_main()
{
    PINKLOG_INFO << "begin..." << std::endl << std::endl;
    
    try{
        test_text();
    }
    catch (const std::string& exp){
        std::cout << exp << std::endl;
    }
    catch (...){
        std::cout << "unknown exception" << std::endl;
    }

    PINKLOG_INFO << "end..." << std::endl;
}