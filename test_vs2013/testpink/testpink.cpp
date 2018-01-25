// testpink.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

extern void test_text_main();
extern void test_format_main();
int _tmain(int argc, _TCHAR* argv[])
{
    test_text_main();

    test_format_main();

	return 0;
}

