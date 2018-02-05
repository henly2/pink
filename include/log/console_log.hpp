/** 
 * @file console_log.hpp
 * @brief 
 * @sa null
 * @author liuheng
 * @date 25/3/2016
 *
 */

#ifndef console_log_hpp_
#define console_log_hpp_

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h> 
#include <stdlib.h>

#include <string>

namespace util
{

#define ON_WM_MYLOG WM_USER+1

class Console_log{
public:
	static Console_log& ins(){
		static Console_log ss;
		return ss;
	}

	bool init(){
		if (my_msg_hwnd_ != NULL && my_con_ != INVALID_HANDLE_VALUE)
		{
			return true;
		}

		if(!InitMessageOnlyWindow())
		{
			return false;
		}

		if(!AllocConsole())
		{
			return false;
		}

		my_con_ = GetStdHandle(STD_OUTPUT_HANDLE);
		if (my_con_ == INVALID_HANDLE_VALUE)
		{
			return false;
		}
		SetConsoleTitleA("Console Log");
		return true;
	}

	void uninit()
	{
		if (my_msg_hwnd_ != NULL)
		{
			::DestroyWindow(my_msg_hwnd_);
			my_msg_hwnd_ = NULL;
		}

		my_con_ = NULL;
		FreeConsole();
	}

	void postLog(const char *format, ... )
	{
		if (my_msg_hwnd_ == NULL)
		{
			return;
		}

		va_list args;
		va_start(args, format);

		SYSTEMTIME time;
		GetLocalTime(&time);
		char szBuffer[1024];
		sprintf_s( szBuffer,"\r\n%02d%02d %02d:%02d:%02d.%03d ",
			time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
		size_t len = strlen(szBuffer);
		int nBuf = _vsnprintf_s(szBuffer+len, 1024-len, _TRUNCATE, format, args);

		va_end(args);

		std::string* lp = new std::string(szBuffer);

		PostMessage(my_msg_hwnd_, ON_WM_MYLOG, 0, (LPARAM)lp);
	}

private:
	Console_log()
		:my_msg_hwnd_(NULL)
		,my_con_(INVALID_HANDLE_VALUE){
	}

	~Console_log(){
	}

private:
	// 创建消息窗口
	BOOL InitMessageOnlyWindow()
	{

		HINSTANCE hInstance = GetModuleHandle(NULL);
		if (hInstance == NULL)
		{
			return FALSE;
		}

		WNDCLASSA wndcls;
		wndcls.cbClsExtra=0;
		wndcls.cbWndExtra=0;
		wndcls.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
		wndcls.hCursor=LoadCursor(NULL,IDC_CROSS);
		wndcls.hIcon=LoadIcon(NULL,IDI_APPLICATION);
		wndcls.hInstance=hInstance;
		wndcls.lpfnWndProc=MessageWinProc;
		wndcls.lpszClassName="Message log Window";
		wndcls.lpszMenuName=NULL;
		wndcls.style=CS_HREDRAW | CS_VREDRAW;
		RegisterClassA(&wndcls);
		my_msg_hwnd_ = CreateWindowExA(0,"Message log Window","Message log Window",0,0,0,0,0,HWND_MESSAGE,0,hInstance,0);
		if (my_msg_hwnd_ == NULL)
		{
			return FALSE;
		}

		return TRUE;
	}
	static LRESULT CALLBACK MessageWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
		{
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case ON_WM_MYLOG:
			{
				std::string* txt = (std::string*)lParam;
				DWORD byteWrited;
                if (Console_log::ins().my_con_ != INVALID_HANDLE_VALUE)
                    WriteConsoleA(Console_log::ins().my_con_, txt->c_str(), txt->size(), &byteWrited, NULL);
                else
                    printf(txt->c_str());
				delete txt;
			}
			break;
		default:
			return DefWindowProc(hwnd,uMsg,wParam,lParam);
		}
		return 0;
	}

private:
	HWND my_msg_hwnd_;//内部消息窗口

	HANDLE my_con_;
};

}

#endif