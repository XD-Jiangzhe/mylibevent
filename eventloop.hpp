#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include "headfile.hpp"
#include "epoller.hpp"


class channel;
class epoller;

using ChannelWeakPtr  = std::weak_ptr<channel>;
using channelPtr = std::shared_ptr<channel>;

using channelVec = std::vector<channel*>;
using epollPtr = std::shared_ptr<epoller>;

using Functor = std::function<void()>;

class eventloop
{
    public:
        
        typedef std::shared_ptr<eventloop> EventLoopPtr;

        eventloop();
        void run();
        
        //这里对epoller中关注的fd的操作进行更新
        void update_channel(const channelPtr&);

    private:
        channelVec activeChannels;
        //这里是对应返回的活跃的channels，channels封装了tcp连接
        
        epollPtr epollerPtr;
        //这里封装了实际的IO复用的一些操作的类
        //该类中有实际的关注的事件
};

#endif