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

int async_query_content(const std::string& url, std::string& content)
{
	HttpRequest hr;
	hr.responseonce = true;
	hr.url = url;
	Http_ResponseCallback cb = [&content](const HttpRequest& request, int requestid, const HttpResponse& response) {
		std::cout << response.http_status << std::endl;
		if (response.http_status == 200) {
			content = response.data;
			std::cout << "content:<" << requestid << ">" << content << std::endl;
		}
	};
	// async
	return HttpDispatcher::Inst().AddRequest(hr, cb);
}

bool sync_query_content(const std::string& url, std::string& content)
{
	HttpRequest hr;
	hr.responseonce = true;
	hr.url = url;

	bool ret = false;
	Http_ResponseCallback cb = [&content, &ret](const HttpRequest& request, int requestid, const HttpResponse& response) {
		std::cout << response.http_status << std::endl;
		if (response.http_status == 200) {
			content = response.data;
			std::cout << "content:" << content << std::endl;
			ret = true;
		}
		else {
			ret = false;
		}
	};
	HttpClientSync cltsync(HttpDispatcher::Inst().GetIoService(), hr, 0, cb);
	cltsync.Start();
	
	return ret;
}

bool sync_download_file(const std::string& url, const std::string& path)
{
	HttpRequest hr;
	hr.responseonce = false;
	hr.url = url;

	bool ret = false;
	Http_ResponseCallback cb = [&path, &ret](const HttpRequest& request, int requestid, const HttpResponse& response) {
		//std::cout << response.http_status << std::endl;
		if (response.http_status == 200) {
			std::cout << "download fin" << std::endl;
			ret = true;
		}
		else if (response.http_status == 206) {
			// write to file
			std::ofstream file;
			if (response.data_times == 1) {
				file.open(path, std::ios_base::out | std::ios_base::binary);
			}
			else {
				file.open(path, std::ios_base::out | std::ios_base::binary | std::ios_base::app);
			}

			file.write(response.data.c_str(), response.data.length());
			file.close();
		}
		else {
			std::cout << "download error:" << response.errmsg << std::endl;
			ret = false;
		}
	};
	HttpClientSync cltsync1(HttpDispatcher::Inst().GetIoService(), hr, 0, cb);
	cltsync1.Start();

	return ret;
}

int async_download_file(const std::string& url, const std::string& path)
{
	HttpRequest hr;
	hr.responseonce = false;
	hr.url = url;

	Http_ResponseCallback cb = [path](const HttpRequest& request, int requestid, const HttpResponse& response) {
		//std::cout << response.http_status << std::endl;
		if (response.http_status == 200) {
			std::cout << "download fin<" << requestid << ">" << std::endl;
		}
		else if (response.http_status == 206) {
			// write to file
			std::ofstream file;
			if (response.data_times == 1) {
				file.open(path, std::ios_base::out | std::ios_base::binary);
			}
			else {
				file.open(path, std::ios_base::out | std::ios_base::binary | std::ios_base::app);
			}
			
			file.write(response.data.c_str(), response.data.length());
			file.close();
		}
		else {
			std::cout << "download error:" << response.errmsg << std::endl;
		}
	};
	return HttpDispatcher::Inst().AddRequest(hr, cb);
}

std::string g_content;
void test_http()
{
	// query content
	std::string url1 = "http://120.27.29.217:82/";
	std::string url2 = "http://120.27.29.217:82/testdownload.zip";
	std::string url3 = "http://120.27.29.217:82/rk.png";

	for (int i = 1; i < 10; i++) {
		async_query_content(url1, g_content);
		async_download_file(url2, std::string("tmp\\") + std::to_string(i) + ".zip");
		async_download_file(url3, std::string("tmp\\") + std::to_string(i) + ".png");
	}
	/*
	std::string content;
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
	HttpDispatcher::Inst().Start(5);
}
void test_http_main_end()
{
	HttpDispatcher::Inst().Stop(true);
}