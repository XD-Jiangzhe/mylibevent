#include "eventloop.hpp"

eventloop::eventloop()
    :epollerPtr(std::make_shared<epoller>(this))
{}

void eventloop::update_channel(const channelPtr& channel)
{
    epollerPtr->updateChannel(channel);    
}

void eventloop::run()
{
    while(1)
    {
        //获取当前的事件，并对返回的活跃的事件进行处理
        //首先清空之前的活跃的事件
        
        activeChannels.clear();
        epollerPtr->do_epoll(&activeChannels);
        
        #ifdef DEBUG
            std::cout<<__FUNCTION__<<" "<<activeChannels.size()<<std::endl;
        #endif
        
        for(auto const i: activeChannels)
        {
            i->handleEvent();
        }
    }
}




