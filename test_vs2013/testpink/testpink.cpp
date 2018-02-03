// testpink.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <conio.h>

extern void test_text_main();
extern void test_format_main();
extern void test_spliter_main();

extern void test_http_main_begin();
extern void test_http_main_end();
extern void test_http_main();
int _tmain(int argc, _TCHAR* argv[])
{
	test_http_main_begin();

	bool quit = false;
	while (!quit) {

		int ch = _getch();
		switch (ch)
		{
		case 'q':
			quit = true;
			break;
		case '1':
			test_http_main();
			break;
		default:
			//test_text_main();
			//test_format_main();
			test_spliter_main();
			break;
		}		
	}

	test_http_main_end();

	return 0;
}

