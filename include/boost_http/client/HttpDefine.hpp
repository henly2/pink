#ifndef SSAsioDataDef_h__
#define SSAsioDataDef_h__

#include <string>

#include <boost/function.hpp> 

namespace util{
    // [[ http 
    // request
    struct HttpRequest
    {
        std::string url;       // url
        std::string postdata;   // if get, is empty, if post, not empty

        HttpRequest()
        {
        }
    };
    // response
    struct HttpResponse
    {
        int http_status;            // http status
        std::string errmsg;         // err message
        std::string header;         // http header
        std::string body;           // http body

        HttpResponse()
            : http_status(0)
        {

        }
    };
    // ( request, requestid, response )
    typedef boost::function<void(const HttpRequest&, int, const HttpResponse&)> SSHttp_Callback;
    // ]] end http

} // namespace util

#endif // SSAsioDataDef_h__
