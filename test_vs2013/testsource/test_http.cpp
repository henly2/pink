#include "stdafx.h"

#include <time.h>
#include <iostream>
#include <sstream>
#include <fstream>

#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
#include "../include/boost_http/client/HttpExport.hpp"
#include "../include/boost_http/util/SimpleUri.hpp"

using namespace boost_http::util;
using namespace boost_http::client;



static std::string get_pathname(const std::string& path)
{
    std::string name(path);

    size_t pos = path.find_last_of('/');
    if (pos != std::string::npos)
    {
        name = path.substr(pos, path.length() - pos);
    }

    return name;
}

//////////////////////////////////////////////////////////////////////////
void test_uri()
{
	const char* urls[] = {
		"http://baidu.com",
		"www.baidu.com",
		"http://www.baidu.com:80",
		"http://12.34.34.34.com:/hello",
		"http://12.34.34.34.com/hello/my",
		"http://12.34.34.34.com:8080/hello/my",
		"http://12.34.34.34.com/hello/my?pp=1",
		"12.34.34.34.com:80/hello/my?pp=1"
	};

	SimpleUri uri;
	for (int i = 0; i < _countof(urls); i++) {
		uri.parse(urls[i]);

		std::cout << "url:" << uri.url() << "\n";
		std::cout << "protocal:" << uri.protocol() << "\n";
		std::cout << "host:" << uri.host() << "\n";
		std::cout << "port:" << uri.port() << "\n";
		std::cout << "path:" << uri.path() << "\n\n";
		
	}

}

//////////////////////////////////////////////////////////////////////////
DWORD WINAPI upload1(LPVOID lpParam)
{
    const char* paths[] = {
        "/yezi/css/entry/images/obq5cu4g00.png",
        "/yezi/css/entry/images/p4906ndnnk.jpg",
        "/yezi/css/entry/images/obq8m0ensw.png"
    };
    int count = _countof(paths);
    int i = 0;
    std::string url = "172.16.8.140:8088";
    std::string dir = "d:\\tmp";
    while (i < count)
    {
        
        std::string urlpath = url + paths[i++];
        std::string path = dir + get_pathname(urlpath);

        async_download_file(urlpath, path);
    }

    return 0;
}

DWORD WINAPI upload11(LPVOID lpParam)
{
    const char* paths[] = {
        "/yezi/css/entry/images/obq5cu4g00.png",
        "/yezi/css/entry/images/p4906ndnnk.jpg",
        "/yezi/css/entry/images/obq8m0ensw.png"
    };
    int count = _countof(paths);
    int i = 0;
    std::string url = "172.16.66.35:8088";
    std::string dir = "d:\\tmp";
    while (i < count)
    {

        std::string urlpath = url + paths[i++];
        std::string path = dir + get_pathname(urlpath);

        async_download_file(urlpath, path);
    }

    return 0;
}

DWORD WINAPI upload2(LPVOID lpParam)
{
    int i = 0;
    while (i++ < 10)
    {
        HttpRequest hr;
        hr.url = "qa.east.idbhost.com/myapi/ic/listqb";

        Http_ResponseCallback cb = [](const HttpRequest& request, int requestid, const HttpResponse& response)
        {
            std::cout << "Content(" << response.http_status << "):" << response.data << "\n";
        };
        HttpDispatcher::Inst().AddRequest(hr, cb);
    }

    return 0;
}

std::string g_content;
void test_http()
{
	// query content
	std::string url1 = "http://120.27.29.217:82/";
	std::string url2 = "http://120.27.29.217:82/testdownload.zip";
	std::string url3 = "http://120.27.29.217:82/rk.png";

	std::string content;
	for (int i = 1; i < 10; i++) {
		async_query_content(url1, g_content);
		async_download_file(url2, std::string("tmp\\") + std::to_string(i) + ".zip");
		async_download_file(url3, std::string("tmp\\") + std::to_string(i) + ".png");
	}
	/*
	sync_query_content(url1, content);
	sync_download_file(url2, "d:\\heng\\tmp.zip");
	sync_download_file(url3, "d:\\heng\\tmp.png");
	*/
}

//////////////////////////////////////////////////////////////////////////
void test_http_main2(const std::string& line)
{
	if (line.size() < 2)
		return;

	std::string p = line.substr(1, line.length() - 1);
	HttpDispatcher::Inst().CancelRequest(atoi(p.c_str()));

	std::cout << "cancel:" << p << std::endl;

}
void test_http_main()
{
    std::cout << "begin..." << std::endl << std::endl;
    
    try{
		//test_uri();
		//test_http();
        HANDLE h1 = CreateThread(NULL, NULL, upload1, NULL, 0, NULL);
        HANDLE h11 = CreateThread(NULL, NULL, upload11, NULL, 0, NULL);
        //HANDLE h2 = CreateThread(NULL, NULL, upload2, NULL, 0, NULL);
    }
    catch (const std::string& exp){
        std::cout << exp << std::endl;
    }
    catch (...){
        std::cout << "unknown exception" << std::endl;
    }

	std::cout << "end..." << std::endl;
}
void test_http_main_begin()
{
    LOGINFO("test_http_main_begin1...");
	HttpDispatcher::Inst().Start(3);
    LOGINFO("test_http_main_begin2...");
}
void test_http_main_end()
{
    LOGINFO("test_http_main_end1...");
	HttpDispatcher::Inst().Stop(false);
    LOGINFO("test_http_main_end2...");
}