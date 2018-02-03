#ifndef HttpClientSync_h__
#define HttpClientSync_h__

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include <boost/thread.hpp> 
#include <boost/noncopyable.hpp> 
#include <boost/function.hpp> 
#include <boost/enable_shared_from_this.hpp>

#include "HttpDefine.hpp"
#include "../util/SimpleUri.hpp"

using boost::asio::ip::tcp;

namespace boost_http {
	namespace client {
		// boost sync http client base asio 
		class HttpClientSync
			: public boost::enable_shared_from_this<HttpClientSync>
		{
		public:
			HttpClientSync(
				boost::asio::io_service& io_service,
				const HttpRequest& http_request,
				int request_id = 0,
				Http_ResponseCallback response_cb = nullptr)
				:resolver_(io_service),
				socket_(io_service),
				http_request_(http_request),
				request_id_(request_id),
				response_cb_(response_cb),
				uri_(http_request.url)
			{
			}

			void Start()
			{
				try {

					//tcp::resolver resolver(io_service);
					tcp::resolver::query query(uri_.host(), uri_.port());

					tcp::resolver::iterator endpoint_iterator = resolver_.resolve(query);
					tcp::resolver::iterator end;

					// Try each endpoint until we successfully establish a connection.
					tcp::socket& socket = socket_;
					boost::system::error_code error = boost::asio::error::host_not_found;
					while (error && endpoint_iterator != end)
					{
						socket.close();
						socket.connect(*endpoint_iterator++, error);
					}
					if (error)
						throw boost::system::system_error(error);

					// Form the request. We specify the "Connection: close" header so that the
					// server will close the socket after transmitting the response. This will
					// allow us to treat all data up until the EOF as the content.
					boost::asio::streambuf request;
					////std::ostream request_stream(&request);
					////request_stream << "GET " << argv[2] << " HTTP/1.0\r\n";
					////request_stream << "Host: " << argv[1] << "\r\n";
					////request_stream << "Accept: */*\r\n";
					////request_stream << "Connection: close\r\n\r\n";

					if (http_request_.postdata.empty()) {
						// get
						build_get_request(request);
					}
					else
					{
						// post
						build_post_request(request, http_request_.postdata);
					}

					// Send the request.
					boost::asio::write(socket_, request);

					// Read the response status line.
					boost::asio::streambuf response;
					response.prepare(1024);
					boost::asio::read_until(socket, response, "\r\n");

					// Check that response is OK.
					std::istream response_stream(&response);
					std::string http_version;
					response_stream >> http_version;
					unsigned int status_code;
					response_stream >> status_code;
					std::string status_message;
					std::getline(response_stream, status_message);
					if (!response_stream || http_version.substr(0, 5) != "HTTP/")
					{
						////std::cout << "Invalid response\n";
						callback(0, "Invalid response");
						return;
					}
					if (status_code != 200)
					{
						////std::cout << "Response returned with status code " << status_code << "\n";
						callback(status_code, "Error response");
						return;
					}

					// Read the response headers, which are terminated by a blank line.
					boost::asio::read_until(socket, response, "\r\n\r\n");

					// Process the response headers.
					std::string header;
					while (std::getline(response_stream, header) && header != "\r") {
						////std::cout << header << "\n";
						http_response_.header += header;
					}
					////std::cout << "\n";

					// Write whatever content we already have to output.
					if (response.size() > 0) {
						////std::cout << &response;
						std::stringstream ss;
						ss << &response;
						http_response_.data = ss.str();
						if (!http_request_.responseonce) {
							callback(206, "");
						}
					}

					// Read until EOF, writing data to output as we go.
					while (boost::asio::read(socket, response,
						boost::asio::transfer_at_least(1), error)) {
						////std::cout << &response;
						std::stringstream ss;
						ss << &response;
						if (!http_request_.responseonce) {
							http_response_.data = ss.str();
							callback(206, "");
						}
						else {
							http_response_.data += ss.str();
						}

					}
					if (error != boost::asio::error::eof)
						throw boost::system::system_error(error);

					callback(200, "");
				}
				catch (const boost::system::system_error& err)
				{
					////std::cout << "Exception1: " << err.code().message() << "\n";
					callback(err.code().value(), err.code().message());
				}
				catch (std::exception& e)
				{
					////std::cout << "Exception2: " << e.what() << "\n";
					callback(0, e.what());
				}

				return;
			}

		private:
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
				++http_response_.data_times;
				http_response_.http_status = status;
				http_response_.errmsg = msg;

				if (!response_cb_.empty())
					response_cb_(http_request_, request_id_, http_response_);
			}

		private:
			tcp::resolver resolver_;
			tcp::socket socket_;

			util::SimpleUri uri_;

			// request id
			int request_id_;

			// request
			HttpRequest http_request_;

			// response
			HttpResponse http_response_;

			// response callback(can be nullptr)
			Http_ResponseCallback response_cb_;
		};
		typedef boost::shared_ptr<HttpClientSync> HttpClientSync_Ptr;

	} // namespace client
} // namespace boost_http

#endif // HttpClientSync_h__