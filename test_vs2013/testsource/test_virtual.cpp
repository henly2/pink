#include "stdafx.h"

#include <string>

// û��������������ͷ�������
class MyInterface{
public:
    virtual ~MyInterface(){}
    virtual void A() = 0;
    virtual void B() = 0;
};

class MyClass : public MyInterface{
public:
    MyClass()
    {
        aa.append(100, 'a');
    }
    virtual ~MyClass()
    {

    }
    virtual void A(){}
    virtual void B(){}
    void C(){};
    void D(){};

private:
    std::string aa;
};

void test_virtual_main()
{
    MyInterface* pp = new MyClass;

    delete pp;
}