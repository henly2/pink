#include "stdafx.h"

#include <time.h>
#include <iostream>
#include <sstream>
#include <fstream>

#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
#include "../include/boost_http/client/HttpDispatcher.hpp"
#include "../include/boost_http/util/SimpleUri.hpp"

using namespace boost_http::util;
using namespace boost_http::client;
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

void test_http()
{
	HttpRequest hr;
	// get content
	hr.responseonce = true;
	hr.url = "http://120.27.29.217:82/";
	Http_ResponseCallback cb = [](const HttpRequest& request, int requestid, const HttpResponse& response) {
		std::cout << response.http_status << std::endl;
		if (response.http_status == 200) {
			//std::cout << response.header << std::endl;
			std::cout << response.data << std::endl;
		}
		else {
			std::cout << response.errmsg << std::endl;
		}
	};
	// async
	HttpDispatcher::Inst().AddRequest(hr, cb);

	// sync
	//HttpClientSync cltsync(HttpDispatcher::Inst().GetIoService(), hr, 0, cb);
	//cltsync.Start();

	// download file
	hr.responseonce = false;
	hr.url = "http://120.27.29.217:82/testdownload.zip";
	//hr.url = "http://120.27.29.217:82/rk.png";
	Http_ResponseCallback cb1 = [](const HttpRequest& request, int requestid, const HttpResponse& response) {
		//std::cout << response.http_status << std::endl;
		if (response.http_status == 200) {
			//std::cout << response.header << std::endl;
			//std::cout << response.body << std::endl;

			// write to file
			//std::ofstream file("D:\\heng\\tmp.png", std::ios_base::out | std::ios_base::binary);

			//file << response.data;
			//std::cout << "write(" << response.data.size() << ")" << std::endl;
		}
		else if (response.http_status == 206) {
			//std::stringstream ss;
			//ss << &response.body_stream;
			//std::cout << "(" << response.body_stream.size() << ")" << ss.str() << std::endl;

			// write to file
			std::ofstream file("D:\\heng\\tmp.zip", std::ios_base::out | std::ios_base::binary | std::ios_base::app);

			file.write(response.data.c_str(), response.data.length());
			file.close();
			//std::cout << "write(" << response.data.length() << ")" << std::endl;
		}
		else {
			std::cout << response.errmsg << std::endl;
		}
	};
	// async
	//HttpDispatcher::Inst().AddRequest(hr, cb1);

	// sync
	HttpClientSync cltsync1(HttpDispatcher::Inst().GetIoService(), hr, 0, cb1);
	cltsync1.Start();
}

//////////////////////////////////////////////////////////////////////////
void test_http_main()
{
    std::cout << "begin..." << std::endl << std::endl;
    
    try{
		//test_uri();
		test_http();
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
	HttpDispatcher::Inst().Start(2);
}
void test_http_main_end()
{
	HttpDispatcher::Inst().Stop(true);
}