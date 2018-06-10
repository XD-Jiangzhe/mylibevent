#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "headfile.hpp"
#include "eventloop.hpp"


class eventloop;
class tcpServer;

//channel是一个包裹，对Tcpconnection和定时timer的封装
//

class channel: public std::enable_shared_from_this<channel>
{
    public:
        using channelPtr = std::shared_ptr<channel>;

        using MessageCallBack = std::function
            <void(channelPtr , char* buffer_, int len)>;
        using EventCallBack = std::function<void()>;
        
        enum
        {
            NoneEvent = 0,
            ReadEvent  = EPOLLIN,
            WriteEvent = EPOLLOUT,
            ErrEvent = EPOLLERR,
        };

        channel(eventloop*, int fd, tcpServer* t=nullptr);
        ~channel()
        {
            #ifdef DEBUG
                std::cout<<"~channel destroy"<<std::endl;
            #endif
        };

        void setReadCallBack(const EventCallBack& rc)
        {readcallback_ = rc;}

        void setWriteCallBack(const EventCallBack& wc)
        {writecallback_ = wc;}

        void setCloseCallBack(const EventCallBack& cc)
        {closecallback_ = cc;}

        void setErrorCallBack(const EventCallBack& ec)
        {errorcallback_= ec;}

        void setConnectionCallBack(const EventCallBack& ccb)
        {connectioncallback_ = ccb;}

        void setMessageCallBack( MessageCallBack mcb)
        {messagecallback_ = mcb;}

        void setWriteCompleteCallBack(const EventCallBack& wcb)
        {writeCompleteCallBack_ = wcb;}

        //这里epoller使用set_revent来返回该channel上发生的事件
        void set_revent(int event){ revents_=event;}
        //得到期望的事件
        int get_event(){return events_;}

        void enableRead(){  events_ |= ReadEvent;   update();}
        void disableRead(){ events_ &= ~ReadEvent;  update();}
        void enableWrite(){ events_ |= WriteEvent;  update();}
        void disableWrite(){events_ &= ~WriteEvent; update();}
        
        void disableAll(){events_ &= NoneEvent; update();}

        bool canRead(){return events_ & ReadEvent;}
        bool canWrite(){return events_& WriteEvent;}

        void sendMessage(std::string);

        //更新epoller中的events的状态
        void update();

        int getfd(){return fd;}
        int getEvents(){return events_;}

        //主处理函数，用来处理返回的revents_事件
        void handleEvent();

        //发送函数
        int send_bytes(const void* data, int len);


    private:
        void handleRead();
        void handleWrite();
        void handleClose();


    private:
        //channel 的读写，关闭，错误回调
        EventCallBack readcallback_;
        EventCallBack writecallback_;
        EventCallBack closecallback_;
        EventCallBack errorcallback_ = nullptr;
        EventCallBack connectioncallback_ = nullptr;

        //channel的读完数据的回调，以及写完所有数据的回调
        MessageCallBack messagecallback_;
        EventCallBack writeCompleteCallBack_;

        //处理管道的套接字
        const int fd;

        eventloop* base_loop;
        tcpServer* server;

        //管道关注的事件
        int events_;
        //实际的事件
        int revents_;

        //缓冲区字段，这里需要一个缓冲区类，先用string类来代替
        //读缓冲区
        std::string readBuffer_;
        //写缓冲区
        std::string outputBuffer_;
        //读缓冲区的函数（暂时）
        ssize_t readAllFromBuffer(int fd, char* data, size_t size);
};

#endif