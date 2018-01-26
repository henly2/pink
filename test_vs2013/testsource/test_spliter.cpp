#include "stdafx.h"
#include <iostream>
#include "../include/text/spliter.h"
void test_spliter_main()
{
    std::vector<std::string> vec;

    text::spliter_by_return("\r\nabcd\n1234\r678\r\nabce", vec);

    for each (auto var in vec)
    {
        std::cout << var << std::endl;
    }
}