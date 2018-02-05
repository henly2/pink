#ifndef HttpExport_h__
#define HttpExport_h__

#include <string>

#include <boost/function.hpp> 

#include "HttpDispatcher.hpp"

namespace boost_http {
	namespace client {
        // [[ 异步请求方法实例，虚结合自己写的回调
        static int async_query_content(const std::string& url, std::string& content)
        {
            // 需要保证content对象在生命周期
            HttpRequest hr;
            hr.responseonce = true;
            hr.url = url;

            Http_ResponseCallback cb = [&content](const HttpRequest& request, int requestid, const HttpResponse& response) {
                if (response.http_status == 200) {
                    content = response.data;
                    LOGINFO("url(%s) query success: %s", request.url.c_str(), content.c_str());
                }
                else{
                    LOGERR("url(%s) query failed: %d-%s", request.url.c_str(), response.http_status, response.errmsg.c_str());
                }
            };
            // async
            return HttpDispatcher::Inst().AddRequest(hr, cb);
        }
        static int async_download_file(const std::string& url, const std::string& path)
        {
            HttpRequest hr;
            hr.responseonce = false;
            hr.url = url;

            Http_ResponseCallback cb = [path](const HttpRequest& request, int requestid, const HttpResponse& response) {
                if (response.http_status == 200) {
                    // download finish
                    LOGINFO("url(%s) download(%s) success", request.url.c_str(), path.c_str());
                }
                else if (response.http_status == 206) {
                    // download data
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
                    // download error
                    LOGERR("url(%s) download(%s) failed: %d-%s", request.url.c_str(), path.c_str(), response.http_status, response.errmsg.c_str());
                }
            };
            return HttpDispatcher::Inst().AddRequest(hr, cb);
        }
        // ]]

        // [[ 同步请求
        static int sync_query_content(const std::string& url, std::string& content)
        {
            HttpRequest hr;
            hr.responseonce = true;
            hr.url = url;

            int httpcode = 0;
            Http_ResponseCallback cb = [&content, &httpcode](const HttpRequest& request, int requestid, const HttpResponse& response) {
                httpcode = response.http_status;
                if (response.http_status == 200) {
                    content = response.data;
                    LOGINFO("url(%s) query success: %s", request.url.c_str(), content.c_str());
                }
                else{
                    LOGERR("url(%s) query failed: %d-%s", request.url.c_str(), response.http_status, response.errmsg.c_str());
                }
            };
            // sync
            HttpClientSync cltsync(HttpDispatcher::Inst().GetIoService(), hr, 0, cb);
            cltsync.Start();

            return httpcode;
        }

        static int sync_download_file(const std::string& url, const std::string& path)
        {
            HttpRequest hr;
            hr.responseonce = false;
            hr.url = url;

            int httpcode = 0;
            Http_ResponseCallback cb = [&path, &httpcode](const HttpRequest& request, int requestid, const HttpResponse& response) {
                httpcode = response.http_status;
                if (response.http_status == 200) {
                    // download finish
                    LOGINFO("url(%s) download(%) success", request.url, path.c_str());
                }
                else if (response.http_status == 206) {
                    // download data
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
                    // download error
                    LOGERR("url(%s) download(%) failed: %d-%s", request.url, path.c_str(), response.http_status, response.errmsg);
                }
            };
            HttpClientSync cltsync(HttpDispatcher::Inst().GetIoService(), hr, 0, cb);
            cltsync.Start();

            return httpcode;
        }
        // ]]

	} // namespace client
} // namespace boost_http

#endif // HttpExport_h__