/*
//保存触发事件的某个文件描述符相关的数据（与具体使用方式有关）  
typedef union epoll_data {  
    void *ptr;  
    int fd;  
    __uint32_t u32;  
    __uint64_t u64;  
} epoll_data_t;  
 //感兴趣的事件和被触发的事件  
struct epoll_event {  
    __uint32_t events;  
    epoll_data_t data; 
};  

*/
#include "epoller.hpp"

epoller::epoller(eventloop* event_base)
                :event_base_(event_base)
                ,epollfd(epoll_create1(EPOLL_CLOEXEC))
                ,eventList_(16)
{}

epoller::~epoller()
{
    close(epollfd);
}

void epoller::updateEpoll_Event(int operation, const channelPtr& channel)
{
    //这里获取到channel的event事件
    epoll_event event; 
    event.events = channel->getEvents();
    event.data.ptr = channel.get();

    //直接对epollfd中的channel进行操作
    epoll_ctl(epollfd, operation, channel->getfd(), &event);

    #ifdef DEBUG
        std::cout<<"epoller "<<__LINE__<<" operation: "<<operationToString(operation)<<std::endl;
    #endif
}


void epoller::updateChannel(const channelPtr& channel)
{  
    //这里直接fd 对应的channel
    //如果当前没有这个fd，表示是新的链接
    
    auto iter = channels_.find(channel->getfd());
    if(iter == channels_.end())
    {
        updateEpoll_Event(EPOLL_CTL_ADD, channel);
        channels_[channel->getfd()] = channel;
    }
    //如果当前有这个fd，则标为修改它的事件
    else
    {
        if(channel->get_event())
        {
            updateEpoll_Event(EPOLL_CTL_MOD, channel);
        }
        else 
        {
            updateEpoll_Event(EPOLL_CTL_DEL, channel);
            removeChannel(channel);
        }
    }
}

void epoller::removeChannel(const channelPtr& channel)
{
    auto iter = channels_.find(channel->getfd());
    if(iter != channels_.end())
    {
        #ifdef DEBUG
            std::cout<<__FUNCTION__<<" remove channel success !"<<std::endl; 
        #endif

        channels_.erase(iter);
        updateEpoll_Event(EPOLL_CTL_DEL, channel);
    }
    else ;
}

void epoller::do_epoll(channelVec* activeChannels, int timeout)
{
    int activenum = epoll_wait(epollfd, &(*eventList_.begin())
                , eventList_.size(), timeout);
    
    //这里返回有效的epoll_event
    #ifdef DEBUG
        std::cout<<"epoll_wait : "<<__LINE__<<" "<<activenum<<std::endl;
    #endif    
    
    if(activenum > 0)
    {
        dealWithActiveEvents(activenum, activeChannels);
    }

    if(activenum == eventList_.size())
        eventList_.resize(2* eventList_.size());
}

void epoller::dealWithActiveEvents(int activenum, channelVec* activeChannels)
{
    assert(activenum < eventList_.size());
    for(int i=0; i<activenum; ++i)
    {
        channel* oneChannel = static_cast<channel*>(eventList_[i].data.ptr);
    
        #ifdef DEBUG
            std::cout<<__FUNCTION__<<" "<<oneChannel->getfd()<<std::endl;
        #endif
        //这里的epoll_event的第一个参数没有放fd，而是放了epoll_event
        //属于的channel的指针
        
        //设置channel上已经发生的事件
        oneChannel->set_revent(eventList_[i].events);
        
        //构建一个需要处理的channel
        activeChannels->push_back(oneChannel);
    }
}

std::string epoller::operationToString(int operation)
{
    switch(operation)
    {
        case EPOLL_CTL_ADD:
            return "operation add";
        case EPOLL_CTL_MOD:
            return "operation mod";
        case EPOLL_CTL_DEL:
            return "operation del";
    }    
}


