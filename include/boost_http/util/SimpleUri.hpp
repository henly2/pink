#ifndef SSSimpleUri_h__
#define SSSimpleUri_h__

#include <string>
#include <algorithm>
#include <cctype>
#include <functional>

namespace util{
    class SSSimpleUri {
        struct Position
        {
            size_t left, right;
            Position()
                : left(0)
                , right(0){
            }
            int count()const{
                return right - left;
            }
        };

    public:
        SSSimpleUri(){
        }

        SSSimpleUri(const std::string& url)
            :url_(url){
            _parse();
        }

        ~SSSimpleUri()
        {
        }

        void parse(const std::string& url){
            url_ = url;
            _parse();
        }

        const std::string& url()const{
            return url_;
        }

        std::string protocol() const {
            return wrap(protocol_, "http");
        }

        std::string host() const {
            return wrap(host_);
        }

        std::string port() const {
            return wrap(port_, "80");
        }

        std::string path() const {
            return wrap(path_);
        }

    private:

        size_t _split(size_t b_pos, const std::string& pp_end, Position& pp){

            pp.left = b_pos;
            pp.right = b_pos;

            if (pp_end.empty())
            {
                pp.right = url_.length();
            }
            else
            {
                std::string::const_iterator pp_i_b = url_.cbegin() + b_pos;
                std::string::const_iterator pp_i_e = std::search(pp_i_b, url_.cend(),
                    pp_end.cbegin(), pp_end.cend());
                if (pp_i_e != url_.cend())
                {
                    pp.right = std::distance(url_.cbegin(), pp_i_e);

                    if (pp.count()==0)
                        std::cerr << "Err(" << pp_end << "):" << url_ << std::endl;
                }
            }

            return pp.right + (pp.count() > 0 ? pp_end.length() : 0);
        }

        void _parse()
        {
            if (url_.empty())
                return;

            std::transform(url_.begin(), url_.end(), url_.begin(), tolower);

            size_t cur_pos = 0;

            // protocol
            const std::string prot_end("://");// can be none
            cur_pos = _split(cur_pos, prot_end, protocol_);
            
            // host and port
            // :
            const std::string host_end_1(":");
            cur_pos = _split(cur_pos, host_end_1, host_);
            if (host_.count() > 0)
            {
                const std::string host_end_2("/"); // if not, can return
                cur_pos = _split(cur_pos, host_end_2, port_);
                if (port_.count() <= 0){
                    port_.right = url_.size();
                    return;
                }
            }
            else
            {
                const std::string host_end_2("/"); // if not, can return
                cur_pos = _split(cur_pos, host_end_2, host_);
                if (host_.count() <= 0){
                    host_.right = url_.size();
                    return;
                }
            }
            
            // path
            cur_pos = _split(cur_pos, "", path_);
        }

    private:
        std::string wrap(const Position& position, const std::string& default = "")const{
            return position.count() > 0 ? url_.substr(position.left, position.count()) : default;
        }

    private:
        std::string url_;       // http://www.appinf.com:88/sample?example-query#frag

        // 注意：这里只做简单的分割，以满足boost http的使用，有需求自行扩展
        Position protocol_;     // http
        Position host_;         // www.appinfo.com
        Position port_;         // 88
        Position path_;         // sample?example-query#frag
    };

} // namespace util

#endif // SSSimpleUri_h__
