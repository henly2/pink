#ifndef SSAsioHttpClient_h__
#define SSAsioHttpClient_h__

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include <boost/thread.hpp> 
#include <boost/noncopyable.hpp> 
#include <boost/function.hpp> 
#include <boost/enable_shared_from_this.hpp>

#include "../SSAsioHttpDef.hpp"
#include "../SSSimpleUri.hpp"

using namespace boost;
using boost::asio::ip::tcp;

namespace util{
    typedef boost::shared_ptr<HttpRequest> HttpRequest_Ptr;
    // (request_ptr, requestid, response)
    typedef boost::function<void(HttpRequest_Ptr, int, const HttpResponse&)> AsioHttpClient_ResponseCallback;

    // boost http client base asio 
    class AsioHttpClient
        : public boost::enable_shared_from_this<AsioHttpClient>
    {
    public:
        AsioHttpClient(
            boost::asio::io_service& io_service,
            boost::shared_ptr<HttpRequest> http_request_ptr,
            int request_id = 0,
            AsioHttpClient_ResponseCallback response_cb = nullptr)
            :resolver_(io_service),
            socket_(io_service),
            http_request_ptr_(http_request_ptr),
            request_id_(request_id),
            response_cb_(response_cb),
            uri_(http_request_ptr->url)
        {
        }

        void Start()
        {
            //tcp::resolver::query query(host_, "http");
            tcp::resolver::query query(uri_.host(), uri_.port());
            resolver_.async_resolve(query,
                boost::bind(&AsioHttpClient::handle_resolve, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::iterator));
        }

        void Cancel()
        {
            socket_.cancel();
        }

    private:
        void handle_resolve(const boost::system::error_code& err,
            tcp::resolver::iterator endpoint_iterator)
        {
            if (!err)
            {
                // Attempt a connection to the first endpoint in the list. Each endpoint
                // will be tried until we successfully establish a connection.
                tcp::endpoint endpoint = *endpoint_iterator;
                socket_.async_connect(endpoint,
                    boost::bind(&AsioHttpClient::handle_connect, this,
                    boost::asio::placeholders::error, ++endpoint_iterator));
            }
            else
            {
                callback(err.value(), err.message());
            }
        }

        void handle_connect(const boost::system::error_code& err,
            tcp::resolver::iterator endpoint_iterator)
        {
            if (!err)
            {
                if (http_request_ptr_->postdata.empty()){
                    // get
                    build_get_request(request_);
                }
                else
                {
                    // post
                    build_post_request(request_, http_request_ptr_->postdata);
                }

                // The connection was successful. Send the request.
                boost::asio::async_write(socket_, request_,
                    boost::bind(&AsioHttpClient::handle_write_request, this,
                    boost::asio::placeholders::error));
            }
            else if (endpoint_iterator != tcp::resolver::iterator())
            {
                // The connection failed. Try the next endpoint in the list.
                socket_.close();
                tcp::endpoint endpoint = *endpoint_iterator;
                socket_.async_connect(endpoint,
                    boost::bind(&AsioHttpClient::handle_connect, this,
                    boost::asio::placeholders::error, ++endpoint_iterator));
            }
            else
            {
                callback(err.value(), err.message());
            }
        }

        void handle_write_request(const boost::system::error_code& err)
        {
            if (!err)
            {
                // Read the response status line.
                boost::asio::async_read_until(socket_, response_, "\r\n",
                    boost::bind(&AsioHttpClient::handle_read_status_line, this,
                    boost::asio::placeholders::error));
            }
            else
            {
                callback(err.value(), err.message());
            }
        }

        void handle_read_status_line(const boost::system::error_code& err)
        {
            if (!err)
            {
                // Check that response is OK.
                std::istream response_stream(&response_);
                std::string http_version;
                response_stream >> http_version;
                unsigned int status_code;
                response_stream >> status_code;
                std::string status_message;
                std::getline(response_stream, status_message);
                if (!response_stream || http_version.substr(0, 5) != "HTTP/")
                {
                    callback(0, "Invalid response");

                    return;
                }
                if (status_code != 200)
                {                   
                    callback(status_code, "Error response");

                    return;
                }

                // Read the response headers, which are terminated by a blank line.
                boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
                    boost::bind(&AsioHttpClient::handle_read_headers, this,
                    boost::asio::placeholders::error));
            }
            else
            {
                callback(err.value(), err.message());
            }
        }

        void handle_read_headers(const boost::system::error_code& err)
        {
            if (!err)
            {
                // Process the response headers.
                std::istream response_stream(&response_);
                std::string header;
                while (std::getline(response_stream, header) && header != "\r"){
                    http_response_.header += header;
                }

                // Write whatever content we already have to output.
                if (response_.size() > 0){
                    std::stringstream ss;
                    ss << &response_;
                    http_response_.body = ss.str();
                }

                // Start reading remaining data until EOF.
                boost::asio::async_read(socket_, response_,
                    boost::asio::transfer_at_least(1),
                    boost::bind(&AsioHttpClient::handle_read_content, this,
                    boost::asio::placeholders::error));
            }
            else
            {
                callback(err.value(), err.message());
            }
        }

        void handle_read_content(const boost::system::error_code& err)
        {
            if (!err)
            {
                // Write all of the data that has been read so far.
                std::stringstream ss;
                ss << &response_;
                http_response_.body += ss.str();

                // Continue reading remaining data until EOF.
                boost::asio::async_read(socket_, response_,
                    boost::asio::transfer_at_least(1),
                    boost::bind(&AsioHttpClient::handle_read_content, this,
                    boost::asio::placeholders::error));
            }
            else if (err != boost::asio::error::eof)
            {
                callback(err.value(), err.message());
            }
            else
            {
                callback(200, "");
            }
        }

        void build_get_request(boost::asio::streambuf& request)
        {
            std::ostream request_stream(&request);
            
            request_stream << "GET " << uri_.path() << " HTTP/1.0\r\n";
            request_stream << "Host: " << uri_.host() << "\r\n";
            request_stream << "Accept: */*\r\n";
            request_stream << "Connection: close\r\n\r\n";
        }

        void build_post_request(boost::asio::streambuf& request, const std::string& postdata)
        {
            std::ostream request_stream(&request);

            request_stream << "POST " << uri_.path() << " HTTP/1.1 \r\n";
            request_stream << "Host:" << uri_.host() << "\r\n";
            request_stream << "User-Agent: C/1.0";
            request_stream << "Content-Type: application/json; charset=utf-8 \r\n";
            request_stream << "Accept: */*\r\n";
            request_stream << "Content-Length: " << std::to_string(postdata.length()) + "\r\n";
            request_stream << "Connection: close\r\n\r\n";
            request_stream << postdata;
        }

        void callback(int status, const std::string& msg)
        {
            http_response_.http_status = status;
            http_response_.errmsg = msg;

            if (!response_cb_.empty())
                response_cb_(http_request_ptr_, request_id_, http_response_);
        }

    private:
        tcp::resolver resolver_;
        tcp::socket socket_;
        boost::asio::streambuf request_;
        boost::asio::streambuf response_;

        SSSimpleUri uri_;

        // request id
        int request_id_;

        // request ptr
        boost::shared_ptr<HttpRequest> http_request_ptr_;

        // response
        HttpResponse http_response_;

        // response callback(can be nullptr)
        AsioHttpClient_ResponseCallback response_cb_;
    };

    typedef boost::shared_ptr<AsioHttpClient> AsioHttpClient_Ptr;

} // namespace util

#endif // SSAsioHttpClient_h__
