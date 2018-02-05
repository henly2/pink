#ifndef HttpDefine_h__
#define HttpDefine_h__

#include <string>
#include <unordered_map>

#include <boost/function.hpp> 

#define USE_CONSOLE_LOG
#ifdef USE_CONSOLE_LOG
#include <windows.h>
static void _myprintf(const char* lev, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    SYSTEMTIME time;
    GetLocalTime(&time);
    char szBuffer[1024];
    sprintf_s(szBuffer, "%02d%02d %02d:%02d:%02d.%03d [%d][%s]",
        time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, GetCurrentThreadId(), lev);
    size_t len = strlen(szBuffer);
    int nBuf = _vsnprintf_s(szBuffer + len, 1024 - len, _TRUNCATE, format, args);
    
    va_end(args);

    printf(szBuffer);
    printf("\n");
}
#define LOGINFO(format, ...) _myprintf("info", format, ##__VA_ARGS__);
#define LOGDEBUG(format, ...) _myprintf("debug", format, ##__VA_ARGS__);
#define LOGERR(format, ...) _myprintf("err", format, ##__VA_ARGS__);
#else
#endif

namespace boost_http {
	namespace client {
		// status
		struct HttpStatus {
			int code;					// code
			const char* description;	// description
			const char* type;			// type
			HttpStatus() {
				memset(this, 0, sizeof(HttpStatus));
			}
			HttpStatus(int x, const char* y, const char* z) 
				: code(x)
				, description(y)
				, type(z)
			{
			}
		};
		static std::unordered_map<int, HttpStatus> g_http_status;
		static const HttpStatus& GetHttpStatus(int code) {
			auto it = g_http_status.find(code);
			if (it != g_http_status.end()){
				return it->second;
			}

			static HttpStatus unknown_status(0, "unknown", "unknown");
			return unknown_status;
		}

#define XX(x, y, z) g_http_status[x] = HttpStatus(x, y, z);
		static bool define_http_status() {
			// http://wiki.splunk.com/Http_status.csv
			XX(100, "Continue", "Informational");
			XX(101, "Switching Protocols", "Informational");
			XX(200, "OK", "Successful");
			XX(201, "Created", "Successful");
			XX(202, "Accepted", "Successful");
			XX(203, "Non-Authoritative Information", "Successful");
			XX(204, "No Content", "Successful");
			XX(205, "Reset Content", "Successful");
			XX(206, "Partial Content", "Successful");
			XX(300, "Multiple Choices", "Redirection");
			XX(301, "Moved Permanently", "Redirection");
			XX(302, "Found", "Redirection");
			XX(303, "See Other", "Redirection");
			XX(304, "Not Modified", "Redirection");
			XX(305, "Use Proxy", "Redirection");
			XX(307, "Temporary Redirect", "Redirection");
			XX(400, "Bad Request", "Client Error");
			XX(401, "Unauthorized", "Client Error");
			XX(402, "Payment Required", "Client Error");
			XX(403, "Forbidden", "Client Error");
			XX(404, "Not Found", "Client Error");
			XX(405, "Method Not Allowed", "Client Error");
			XX(406, "Not Acceptable", "Client Error");
			XX(407, "Proxy Authentication Required", "Client Error");
			XX(408, "Request Timeout", "Client Error");
			XX(409, "Conflict", "Client Error");
			XX(410, "Gone", "Client Error");
			XX(411, "Length Required", "Client Error");
			XX(412, "Precondition Failed", "Client Error");
			XX(413, "Request Entity Too Large", "Client Error");
			XX(414, "Request-URI Too Long", "Client Error");
			XX(415, "Unsupported Media Type", "Client Error");
			XX(416, "Requested Range Not Satisfiable", "Client Error");
			XX(417, "Expectation Failed", "Client Error");
			XX(500, "Internal Server Error", "Server Error");
			XX(501, "Not Implemented", "Server Error");
			XX(502, "Bad Gateway", "Server Error");
			XX(503, "Service Unavailable", "Server Error");
			XX(504, "Gateway Timeout", "Server Error");
			XX(505, "HTTP Version Not Supported", "Server Error");

			return true;
		}
		static bool _define_http_status = define_http_status();

		// request
		struct HttpRequest
		{
			bool responseonce;		// if true, return by once; else return by each get data by 206 code
			std::string url;		// url
			std::string postdata;   // if get, is empty, if post, not empty

			HttpRequest(bool _responseonce = true)
				: responseonce(_responseonce)
			{
			}
		};
		// response
		struct HttpResponse
		{
			int http_status;					// http status
			std::string errmsg;					// err message
			std::string header;					// http header
			std::string data;					// http data, maybe many time by HttpRequest.responseonce
			int data_times;						// receive data times

			HttpResponse()
				: http_status(0)
				, data_times(0)
			{
			}
		};
		// ( request, requestid, response )
		typedef boost::function<void(const HttpRequest&, int, const HttpResponse&)> Http_ResponseCallback;

	} // namespace client
} // namespace boost_http

#endif // HttpDefine_h__