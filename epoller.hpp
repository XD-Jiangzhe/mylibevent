#ifndef EPOLLER_HPP
#define EPOLLER_HPP

#include "headfile.hpp"
#include "channel.hpp"


class eventloop;
class channel;

using channelPtr = std::shared_ptr<channel>;
using channelWeakPtr = std::weak_ptr<channel>;
using channelVec = std::vector<channel*>;

using eventVec = std::vector<epoll_event>;

class epoller
{
    public:
        using channelMap = std::map<int, channelPtr>;

        explicit epoller(eventloop* event_base);
        ~epoller();

        void updateChannel(const channelPtr& );
        void removeChannel(const channelPtr& );

        void updateEpoll_Event(int operation, const channelPtr&);

        void do_epoll(channelVec*, int timeout=-1);
    
        std::string operationToString(int operation);


    private:
        void dealWithActiveEvents(int acviveChannel, channelVec*);

    private:
        //套接字对应的 channel
        channelMap channels_;
        
        //这里是关注的epoll_event的vector 
        eventVec eventList_;

        int epollfd;
        eventloop* event_base_;
};
#endif