#include "stdafx.h"

#include <iostream>
#include <sstream>

#include <cmath>
#include <string>
using namespace std;
void print_limit()
{
    cout << "type: \t\t" << "************size**************" << endl;
    cout << "bool: \t\t" << "所占字节数：" << sizeof(bool);
    cout << "\t最大值：" << (numeric_limits<bool>::max)();
    cout << "\t\t最小值：" << (numeric_limits<bool>::min)() << endl;
    cout << "char: \t\t" << "所占字节数：" << sizeof(char);
    cout << "\t最大值：" << (numeric_limits<char>::max)();
    cout << "\t\t最小值：" << (numeric_limits<char>::min)() << endl;
    cout << "signed char: \t" << "所占字节数：" << sizeof(signed char);
    cout << "\t最大值：" << (numeric_limits<signed char>::max)();
    cout << "\t\t最小值：" << (numeric_limits<signed char>::min)() << endl;
    cout << "unsigned char: \t" << "所占字节数：" << sizeof(unsigned char);
    cout << "\t最大值：" << (numeric_limits<unsigned char>::max)();
    cout << "\t\t最小值：" << (numeric_limits<unsigned char>::min)() << endl;
    cout << "wchar_t: \t" << "所占字节数：" << sizeof(wchar_t);
    cout << "\t最大值：" << (numeric_limits<wchar_t>::max)();
    cout << "\t\t最小值：" << (numeric_limits<wchar_t>::min)() << endl;
    cout << "short: \t\t" << "所占字节数：" << sizeof(short);
    cout << "\t最大值：" << (numeric_limits<short>::max)();
    cout << "\t\t最小值：" << (numeric_limits<short>::min)() << endl;
    cout << "int: \t\t" << "所占字节数：" << sizeof(int);
    cout << "\t最大值：" << (numeric_limits<int>::max)();
    cout << "\t最小值：" << (numeric_limits<int>::min)() << endl;
    cout << "unsigned: \t" << "所占字节数：" << sizeof(unsigned);
    cout << "\t最大值：" << (numeric_limits<unsigned>::max)();
    cout << "\t最小值：" << (numeric_limits<unsigned>::min)() << endl;
    cout << "long: \t\t" << "所占字节数：" << sizeof(long);
    cout << "\t最大值：" << (numeric_limits<long>::max)();
    cout << "\t最小值：" << (numeric_limits<long>::min)() << endl;
    cout << "unsigned long: \t" << "所占字节数：" << sizeof(unsigned long);
    cout << "\t最大值：" << (numeric_limits<unsigned long>::max)();
    cout << "\t最小值：" << (numeric_limits<unsigned long>::min)() << endl;
    cout << "double: \t" << "所占字节数：" << sizeof(double);
    cout << "\t最大值：" << (numeric_limits<double>::max)();
    cout << "\t最小值：" << (numeric_limits<double>::min)() << endl;
    cout << "long double: \t" << "所占字节数：" << sizeof(long double);
    cout << "\t最大值：" << std::to_string((numeric_limits<long double>::max)()/2);
    cout << "\t最小值：" << std::to_string((numeric_limits<long double>::min)()) << endl;
    cout << "float: \t\t" << "所占字节数：" << sizeof(float);
    cout << "\t最大值：" << std::to_string((numeric_limits<float>::max)());
    cout << "\t最小值：" << std::to_string((numeric_limits<float>::min)()) << endl;
    cout << "size_t: \t" << "所占字节数：" << sizeof(size_t);
    cout << "\t最大值：" << (numeric_limits<size_t>::max)();
    cout << "\t最小值：" << (numeric_limits<size_t>::min)() << endl;
    cout << "string: \t" << "所占字节数：" << sizeof(string) << endl;
    // << "\t最大值：" << (numeric_limits<string>::max)() << "\t最小值：" << (numeric_limits<string>::min)() << endl;
    cout << "type: \t\t" << "************size**************" << endl;
}

void test_format_main()
{
    /*
    使用ostringstream时，会自动将大数据格式化成科学计数法；
    使用std::to_string(),但是会有默认补零的缺陷；
    使用"%.15lg"格式化，可以增大变成科学计数法的数值范围，且可去掉无效的0；
    */
    print_limit();


    std::cout << "c:" << std::to_string(float(20141232.0)) << std::endl;
    //return;

    int a = 1000;
    float b = 1000000.3782;
    double c = 100000000000000.849;
    
    char buf[64] = { 0 };
    _snprintf_s(buf, _countof(buf), _TRUNCATE, "%d", a);
    std::cout << "a:" << buf << std::endl;

    _snprintf_s(buf, _countof(buf), _TRUNCATE, "%f", b);
    std::cout << "b:" << buf << std::endl;
    _snprintf_s(buf, _countof(buf), _TRUNCATE, "%.15lg", b);
    std::cout << "b:" << buf << std::endl;
    std::ostringstream ss;
    ss << b;
    std::cout << "b:" << ss.str() << std::endl;
    std::cout << "b:" << std::to_string(b) << std::endl;

    _snprintf_s(buf, _countof(buf), _TRUNCATE, "%.15f", c);
    std::cout << "c:" << buf << std::endl;
    _snprintf_s(buf, _countof(buf), _TRUNCATE, "%.15lg", c);
    std::cout << "c:" << buf << std::endl;

    std::ostringstream ss1;
    ss1 << c;
    std::cout << "c:" << ss1.str() << std::endl;
    std::cout << "c:" << std::to_string(c) << std::endl;
    
}