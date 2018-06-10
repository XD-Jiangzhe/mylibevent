#include "tcpserver.hpp"

using namespace std::placeholders;

tcpServer::tcpServer(eventloop* loop, std::string IpAddr, int port, int listenSize, int threadnum)
                    :base_loop(loop)
                    ,listener_(std::make_shared<listener>(loop, IpAddr,port, listenSize))
                    ,threadsPtr_(std::make_shared<ThreadPool>(base_loop, threadnum))
{
    //这里绑定了监听器的 新链接的回调函数
    auto callBack =[this](eventloop* event_base, int fd)
    {
        defaultConnectionCB(event_base, fd);
    };

    listener_->setNewConnectionCallBack(callBack);
}

void tcpServer::start()
{
    threadsPtr_->run();
    base_loop->run();
}

//这里默认的每来一个链接，就将该链接封装成channel 放到epoller中去
void tcpServer::defaultConnectionCB(eventloop* event_base, int fd)
{
    //这里创建一个新的线程
    channelPtr newChannelPtr  = std::make_shared<channel>(base_loop, fd, this);
    
    //绑定到线程池中
    newChannelPtr->setMessageCallBack(messagecallback_);
    newChannelPtr->setWriteCallBack(writeCompleteCallBack_);
    
    //将新的channel 放到里面去
    base_loop->update_channel(newChannelPtr);
    newChannelPtr->enableRead();
} 