#ifndef SimpleUri_h__
#define SimpleUri_h__

#include <string>
#include <algorithm>
#include <cctype>
#include <functional>

using std::string;

namespace boost_http {
	namespace util {
		static const string protocol_tag = "://";
		static const string colon_tag = ":";
		static const string slash_tag = "/";
		class SimpleUri {
			struct Position
			{
				size_t left, right;
				Position()
					: left(0)
					, right(0) {
				}
				int count()const {
					return right - left;
				}
				void reset() {
					left = 0;
					right = 0;
				}
			};
			// 注意：这里只做简单的分割，以满足boost http的使用，有需求自行扩展
			Position protocol_;     // http
			Position host_;         // www.appinfo.com
			Position port_;         // 88
			Position path_;         // sample?example-query#frag

			string url_;       // http://www.appinf.com:88/sample?example-query#frag

		public:
			SimpleUri() {
			}

			SimpleUri(const string& url)
				:url_(url) {
				_parse();
			}

			~SimpleUri()
			{
			}

			void parse(const string& url) {
				url_ = url;
				reset();
				_parse();
			}

			const string& url()const {
				return url_;
			}

			string protocol() const {
				return wrap(protocol_, "http");
			}

			string host() const {
				return wrap(host_);
			}

			string port() const {
				return wrap(port_, "80");
			}

			string path() const {
				return wrap(path_, "/");
			}

		private:

			bool _split(size_t b_pos, const string& pp_end, Position& pp, bool notfindtoend = false) {

				pp.left = b_pos;
				pp.right = b_pos;

				if (pp_end.empty())
				{
					pp.right = url_.length();
				}
				else
				{
					string::const_iterator pp_i_b = url_.cbegin() + b_pos;
					string::const_iterator pp_i_e = std::search(pp_i_b, url_.cend(),
						pp_end.cbegin(), pp_end.cend());
					if (pp_i_e != url_.cend()){
						pp.right = std::distance(url_.cbegin(), pp_i_e);
					}
					else if (notfindtoend) {
						pp.right = url_.length();
					}
				}

				// end
				if (pp.right == url_.length()) {
					return true;
				}

				return false;
			}

			void _parse()
			{
				if (url_.empty())
					return;

				std::transform(url_.begin(), url_.end(), url_.begin(), tolower);

				size_t cur_pos = 0;

				// protocol
				if (_split(cur_pos, protocol_tag, protocol_))
					return;
				if (protocol_.count() > 0) {
					// find protocol
					cur_pos += protocol_.count() + protocol_tag.length();
				}

				// host and port
				if (_split(cur_pos, colon_tag, host_))
					return;
				if (host_.count() > 0)
				{
					// find host by colon, find port
					cur_pos += host_.count() + colon_tag.length();
					if (_split(cur_pos, slash_tag, port_, true))
						return;

					cur_pos += port_.count();
				}
				else
				{
					// find host by slash, no port
					if (_split(cur_pos, slash_tag, host_, true))
						return;

					cur_pos += host_.count();
				}

				// path
				_split(cur_pos, "", path_);
				
			}

		private:
			string wrap(const Position& position, const string& default = "")const {
				return position.count() > 0 ? url_.substr(position.left, position.count()) : default;
			}

			void reset() {
				protocol_.reset();
				host_.reset();
				port_.reset();
				path_.reset();
			}
		};

	} // namespace util
} // namespace boost_http

#endif // SimpleUri_h__
