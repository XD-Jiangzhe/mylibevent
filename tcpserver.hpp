#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include "listener.hpp"
#include "eventloop.hpp"
#include "thread_pool.hpp"
#include "headfile.hpp"

using namespace jz;

class channel;
class eventloop;

class tcpServer
{
    public:
        using MessageCallBack = channel::MessageCallBack;
        // using MessageCallBack =std::function<void(tcpServer*, channelPtr, char*, int len)>;
        using EventCallBack = channel::EventCallBack;
        using threadsPtr = std::shared_ptr<ThreadPool>;

        tcpServer(eventloop* loop, std::string IpAddr, int port, int listenSize, int threadPoolSize =0);
        tcpServer(eventloop* loop, sockaddr_in sock, int listenSize, int threadPoolSize =0);

        void defaultConnectionCB(eventloop* event_base, int fd);        
        
        void setMessageCallBack(const MessageCallBack& mcb)
        {
            messagecallback_ = mcb;
        }

        void setEventCallBack(const EventCallBack& wccb)
        {writeCompleteCallBack_ = wccb;}

        void start();
        
        threadsPtr getThreadPool(){ return threadsPtr_;}

        template<typename Function, typename... Args>
        //放在形参的左边 声明一个形参包,在形式参数游标,表示将形参包解为各自独立的实参
        void addToThreadPool(Function&& func, Args&& ...args)
        {
            std::future< typename std::result_of<Function(Args...)>::type>
            fu = threadsPtr_->add(std::forward<Function>(func), std::forward<Args>(args)...);
            fu.get();
        }

    private:
        eventloop* base_loop;
        std::shared_ptr<listener> listener_;
        
        connection_cb connectionCallBack_;
        
        MessageCallBack messagecallback_;
        channel::EventCallBack writeCompleteCallBack_;

        std::shared_ptr<ThreadPool> threadsPtr_ ;

};

#endif