#ifndef HttpDispatcher_h__
#define HttpDispatcher_h__

#include <algorithm>
#include <vector>
#include <unordered_map>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/atomic.hpp>

#include <boost/thread.hpp> 
#include <boost/noncopyable.hpp> 
#include <boost/function.hpp> 
#include <boost/enable_shared_from_this.hpp>

#include "HttpClient.hpp"
#include "HttpClientSync.hpp"

namespace boost_http {
	namespace client {
		// Http调度器
		class HttpDispatcher {
			// user information
			struct UserInfo
			{
				// request id
				int request_id;
				// response callback
				Http_ResponseCallback response_cb;

				UserInfo()
					: request_id(0)
				{

				}
			};

		public:
			static HttpDispatcher& Inst()
			{
				static HttpDispatcher hd;
				return hd;
			}

			bool Start(unsigned int threadnum = 2)
			{
				thread_num_ = threadnum;
				LOGINFO("HttpDispatcher Start(%d) thread begin", thread_num_);

				bstop_ = false;

				for (size_t i = 0; i < thread_num_; i++)
				{
					thread_vec_.push_back(boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&HttpDispatcher::OnRun, this))));
				}
				/*
				for (size_t i = 0; i < thread_num_; i++)
				{
					thread_vec_.push_back(
						boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_))));
				}*/

                LOGINFO("HttpDispatcher Start(%d) thread end", thread_num_);

				return thread_vec_.size() > 0;
			}

			void Stop(bool needwait = true)
			{
                LOGINFO("HttpDispatcher Stop(%d) begin", needwait);
                if (bstop_){
                    LOGERR("HttpDispatcher already Stop");
                    return;
                }

				bstop_ = true;
                bwait_ = needwait;

                if (!bwait_)
                {
                    _CancelUserAll();
                    _CancelRequestAll();
                    _CancelClientAll();

                    io_service_.stop();
                }
                else
                {
                    _RestartTimer(100);
                }

                for (auto it = thread_vec_.cbegin(); it != thread_vec_.cend(); ++it)
                {
                    (*it)->join();
                }
                thread_vec_.clear();

                LOGINFO("HttpDispatcher Stop(%d) end", needwait);
			}

			int AddRequest(const HttpRequest& request, Http_ResponseCallback cb)
			{
				if (bstop_)
				{
                    LOGERR("HttpDispatcher is Stopped...");
					return 0;
				}

				int requestid = ++increase_id_;

				_AddUser(requestid, cb);
				_AddRequest(requestid, request);

				if (_CountClient() < thread_num_)
					_RestartTimer(100);

                LOGDEBUG("HttpDispatcher add request(%s), requestid = %d", request.url.c_str(), requestid);

				return requestid;
			}

			void CancelRequest(int requestid)
			{
                LOGDEBUG("HttpDispatcher cancel request, requestid = %d", requestid);

				// 删除用户数据信息
				_CancelUser(requestid);

				// 删除等待队列中
				_CancelRequest(requestid);

				// 取消操作
				_CancelClient(requestid);
			}

		public:
			boost::asio::io_service& GetIoService() {
				return io_service_;
			}

		protected:
			HttpDispatcher()
				: thread_num_(1)
				, bstop_(true)
                , bwait_(false)
				, timer_(io_service_)
				, io_work_(io_service_)
				, increase_id_(0)
			{

			}

			~HttpDispatcher()
			{
				_CancelUserAll();
				_CancelRequestAll();
				_CancelClientAll();
				_RemoveClientAll();
			}

			void OnRun()
			{
				LOGDEBUG("HttpDispatcher OnRun begin...");

				try
				{
					//timer_.expires_at(boost::get_system_time() + boost::posix_time::seconds(10 * 1000));
					//timer_.async_wait(boost::bind(&HttpDispatcher::OnTimer, this, boost::asio::placeholders::error));

					//boost::asio::io_service::work w(io_service_);
					io_service_.run();
				}
				catch (std::exception& e)
				{
                    LOGERR("HttpDispatcher Exception: %s", e.what());
				}

                LOGDEBUG("HttpDispatcher OnRun end...");
			}

			void OnTimer(const boost::system::error_code& err)
			{
                //LOGDEBUG("OnTimer begin...");

				// cancel
				if (err)
				{
                    //LOGDEBUG("Timer err, do nothing...");
					return;
				}

				size_t clt_count = _CountClient();
				for (size_t i = clt_count; i < thread_num_; i++)
				{
					// get first item
					HttpRequest request;
					int requestid = _PopFrontRequest(request);

					// dispatch this item
					if (requestid == 0)
					{
						// no more request
						//std::cout << "no http request" << std::endl;
                        LOGDEBUG("no more task");
						break;
					}

					// build&dispatch a task
					HttpClient_Ptr c = _BuildTask(requestid, request);
					_AddClient(requestid, c);
					c->Start();
				}

                if (bstop_)
                {
                    if (bwait_)
                    {
                        if (_CountClient() == 0 && _CountReqeust() == 0 && _CountUser() == 0)
                        {
                            // no request
                            io_service_.stop();
                        }
                    }
                }

				LOGDEBUG("waiting size: %d", _CountReqeust());
				LOGDEBUG("client size: %d", _CountClient());
                LOGDEBUG("user size: %d", _CountUser());

                //LOGDEBUG("OnTimer end...");
			}

			void OnHttpCallback(const HttpRequest& request, int requestid, const HttpResponse& response)
			{
                LOGDEBUG("callback, rid = %d, httpstatus = %d", requestid, response.http_status);

				if (response.http_status == 206 && request.responseonce == false) {
					_CallbackUser(request, requestid, response);
				}
				else {
					_CallbackAndRemoveUser(request, requestid, response);

					_RemoveClient(requestid);

					_RestartTimer(100);
				}
			}

		private:
			inline HttpClient_Ptr _BuildTask(int requestid, const HttpRequest& request_ptr)
			{
				LOGDEBUG("create client, rid = %d", requestid);

				HttpClient_Ptr c(
					new HttpClient(
						io_service_,
						request_ptr,
						requestid,
						boost::bind(&HttpDispatcher::OnHttpCallback, this, _1, _2, _3))
				);

				return c;
			}

			inline void _RestartTimer(int timer_msecond)
			{
				timer_.cancel();

				timer_.expires_at(boost::get_system_time() + boost::posix_time::milliseconds(timer_msecond));
				timer_.async_wait(boost::bind(&HttpDispatcher::OnTimer, this, boost::asio::placeholders::error));
			}

		private:
			// thread num
			unsigned int thread_num_;

			// status
			volatile bool bstop_;
            // need wait all request when quit
            volatile bool bwait_;

			// io service
			boost::asio::io_service io_service_;
			boost::asio::io_service::work io_work_;

			// io service thread
			std::vector<boost::shared_ptr<boost::thread>> thread_vec_;

			// timer
			boost::asio::deadline_timer timer_;

			// users
			boost::mutex mutex_user_;
            std::unordered_map<int, UserInfo> users_;
			// [[
			void _AddUser(int requestid, Http_ResponseCallback cb)
			{
				UserInfo ui;
				ui.request_id = requestid;
				ui.response_cb = cb;

				boost::unique_lock<boost::mutex> lock(mutex_user_);
				users_[requestid] = ui;
			}

			void _CancelUser(int requestid)
			{
				boost::unique_lock<boost::mutex> lock(mutex_user_);
				auto it = users_.find(requestid);
				if (it != users_.end())
				{
					users_.erase(it);
				}
			}

			void _CallbackUser(const HttpRequest& request, int requestid, const HttpResponse& response)
			{
				boost::unique_lock<boost::mutex> lock(mutex_user_);
				auto it = users_.find(requestid);
				if (it != users_.end())
				{
					UserInfo& ui = it->second;
					if (!ui.response_cb.empty())
					{
						ui.response_cb(request, requestid, response);
					}
				}
			}

			void _CallbackAndRemoveUser(const HttpRequest& request, int requestid, const HttpResponse& response)
			{
				boost::unique_lock<boost::mutex> lock(mutex_user_);
				auto it = users_.find(requestid);
				if (it != users_.end())
				{
					UserInfo& ui = it->second;
					if (!ui.response_cb.empty())
					{
						ui.response_cb(request, requestid, response);
					}

					users_.erase(it);
				}
			}

			void _CancelUserAll()
			{
				boost::unique_lock<boost::mutex> lock(mutex_user_);
				users_.clear();
			}

			size_t _CountUser()
			{
				boost::unique_lock<boost::mutex> lock(mutex_user_);
				return users_.size();
			}
			// ]]

			// request
			boost::mutex mutex_request_;
			std::unordered_map<int, HttpRequest> waiting_request_;
			// [[
			void _AddRequest(int requestid, const HttpRequest& request)
			{
				boost::unique_lock<boost::mutex> lock(mutex_request_);
				waiting_request_[requestid] = request;
			}

			int _PopFrontRequest(HttpRequest& request)
			{
				int rid = 0;
				boost::unique_lock<boost::mutex> lock(mutex_request_);
				if (waiting_request_.size() > 0)
				{
					auto it = waiting_request_.begin();

					rid = it->first;
					request = it->second;
					waiting_request_.erase(it);
				}
				return rid;
			}

			void _CancelRequest(int requestid)
			{
				boost::unique_lock<boost::mutex> lock(mutex_request_);
				auto it = waiting_request_.find(requestid);
				if (it != waiting_request_.end())
				{
					waiting_request_.erase(it);
				}
			}

			void _CancelRequestAll()
			{
				boost::unique_lock<boost::mutex> lock(mutex_request_);
				waiting_request_.clear();
			}

			size_t _CountReqeust()
			{
				boost::unique_lock<boost::mutex> lock(mutex_request_);
				return waiting_request_.size();
			}
			// ]]

			// http client
			boost::mutex mutex_client_;
            std::unordered_map<int, HttpClient_Ptr> http_clients_;
			// [[
			void _AddClient(int requestid, HttpClient_Ptr c)
			{
				boost::unique_lock<boost::mutex> lock(mutex_client_);
				http_clients_.insert(std::make_pair(requestid, c));
			}

			void _CancelClient(int requestid)
			{
				boost::unique_lock<boost::mutex> lock(mutex_client_);
				auto it = http_clients_.find(requestid);
				if (it != http_clients_.end())
				{
					it->second->Cancel();
				}
			}

			void _RemoveClient(int requestid)
			{
				boost::unique_lock<boost::mutex> lock(mutex_client_);
				auto it = http_clients_.find(requestid);
				if (it != http_clients_.end())
				{
					http_clients_.erase(it);
				}
			}

			void _CancelClientAll()
			{
				boost::unique_lock<boost::mutex> lock(mutex_client_);
				for (auto it = http_clients_.begin(); it != http_clients_.end(); it++)
				{
					it->second->Cancel();
				}
			}

			void _RemoveClientAll()
			{
				boost::unique_lock<boost::mutex> lock(mutex_client_);
				http_clients_.clear();
			}

			size_t _CountClient()
			{
				boost::unique_lock<boost::mutex> lock(mutex_client_);
				return http_clients_.size();
			}
			// ]]

			// ++id from 0
			boost::atomic_int increase_id_;
		};

	} // namespace util
} // namespace boost_http
#endif // HttpDispatcher_h__
