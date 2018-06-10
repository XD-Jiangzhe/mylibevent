#ifndef LISTENER_HPP
#define LISTENER_HPP

#include "eventloop.hpp"
#include "headfile.hpp"
#include "channel.hpp"

#include <iostream>
#include <string>
#include <memory>

class eventloop;


// typedef void(*listener_cb)();
// typedef void(*connection_cb)(eventloop*, int );
using listener_cb = std::function<void()>;
using connection_cb = std::function<void(eventloop*, int)>;

class listener
{   
    public:
    
        listener(eventloop* base_loop, std::string IpAddr, int port, int listenSize);
        listener(eventloop* base_loop, sockaddr_in sock, int listenSize);

        ~listener()
        {
            close(listenChannel->getfd());
        }

        void setNewConnectionCallBack(connection_cb nccb)
        {newConnectionCallBack = nccb;}

        //这里监听套接字可读，这里建立连接
        void handle_read();

        int  getListenerFd(){return listenChannel->getfd();}

    private:
        void listenInit();

    private:
        eventloop* Loop;
        
        //这里是监听套接字可读的回调函数
        listener_cb readCb;
        //接收到新链接的时候的回调函数
        connection_cb newConnectionCallBack;

        sockaddr_in sock_;
        //监听通道
        channelPtr listenChannel;
};

int listenAndBind(sockaddr_in sock, int listenSize);

#endif