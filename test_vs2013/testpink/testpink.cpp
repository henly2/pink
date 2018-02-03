// testpink.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <windows.h>
#include <conio.h>
#include <iostream>
#include <string>

extern void test_text_main();
extern void test_format_main();
extern void test_spliter_main();

extern void test_http_main_begin();
extern void test_http_main_end();
extern void test_http_main();
extern void test_http_main2(const std::string& line);
int _tmain(int argc, _TCHAR* argv[])
{
	test_http_main_begin();

	std::string line;
	bool quit = false;
	while (!quit) {

		std::cin >> line;
		switch (line[0])
		{
		case 'q':
		case 'p':
			quit = true;
			break;
		case 'c':
			system("cls");
			break;
		case '1':
			test_http_main();
			break;
		case '2':
			test_http_main2(line);
			break;
		default:
			//test_text_main();
			//test_format_main();
			test_spliter_main();
			break;
		}		
	}

	test_http_main_end();

	if (line[0] == 'p')
		system("pause");

	return 0;
}

