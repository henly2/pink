#include "stdafx.h"

#include <functional>

typedef std::tr1::function<void(void*)> XOperator;

// [[ 1
template <typename T>
struct XAskDataOperator1{
    void Release(void* data){
        delete (T*)data;
    }
};

template <typename T>
XOperator CreateBindFunction1(T* data)
{
    return std::tr1::bind(&XAskDataOperator1<T>::Release, XAskDataOperator1<T>(), std::tr1::placeholders::_1);
}
// ]]


// [[ 2
template <typename T>
struct XAskDataOperator2{
    void operator()(void* data){
        delete (T*)data;
    }
};
template <typename T>
XOperator CreateBindFunction2(T* data)
{
    return std::tr1::bind(XAskDataOperator2<T>(), std::tr1::placeholders::_1);
}

// [[ 3
template <typename T>
void DoRelease(void* data)
{
    delete (T*)data;
}

template <typename T>
XOperator CreateBindFunction3(T* data)
{
    return std::tr1::bind(&DoRelease<T>, std::tr1::placeholders::_1);
}
// ]]

//////////////////////////////////////////////////////////////////////////
// 测试代码
// 定义一个void*容器，需要绑定释放函数
struct MyBase{
    MyBase(){
        data = nullptr;
        op = nullptr;
    }
    ~MyBase(){
        clear();
    }

    void set(void* d, XOperator o)
    {
        op = o;
        clear();
        data = d;
    }

    void clear(){
        if (data != NULL)
        {
            if (op != nullptr)
                op(data);
            else
                delete data;

            data = NULL;
        }
    }

    void* data;
    XOperator op;
};

struct MyType1{
    std::string data;
    MyType1(){
        data = "MyType1";
    }
    ~MyType1(){

    }
};
struct MyType2{
    std::string data;
    MyType2(){
        data = "MyType2";
    }
    ~MyType2(){

    }
};
struct MyType3{
    std::string data;
    MyType3(){
        data = "MyType3";
    }
    ~MyType3(){

    }
};

void test_bind_main()
{
    {
        MyBase m1;
        MyType1* ptr = new MyType1;
        //m1.set(ptr, CreateBindFunction1(ptr));
        m1.set(ptr, nullptr);
    }

    {
        MyBase m1;
        MyType2* ptr = new MyType2;
        //m1.set(ptr, CreateBindFunction2(ptr));
        m1.set(ptr, nullptr);
    }

    {
        MyBase m1;
        MyType1* ptr = new MyType1;
        //m1.set(ptr, CreateBindFunction3(ptr));
        m1.set(ptr, nullptr);
    }
}