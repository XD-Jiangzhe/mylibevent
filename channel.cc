#include "channel.hpp"

/*这里调用eventloop的update_channel，
eventloop的update_channel再回调epoller的update来对关注的事件等进行修改
*/

channel::channel(eventloop* loop, int fd, tcpServer* tcpsever_)
                :base_loop(loop), fd(fd), events_(0),revents_(0),server(tcpsever_)
{
    //设置默认的读写 关闭的回调函数
    setReadCallBack(std::bind(&channel::handleRead, this));
    setWriteCallBack(std::bind(&channel::writecallback_, this));
    setCloseCallBack(std::bind(&channel::handleClose, this));
}

void channel::handleEvent()
{
    if((revents_ & ReadEvent) && readcallback_)
    {
        #ifdef DEBUG
            std::cout<<__FUNCTION__<<" socket readlable"<<std::endl;
        #endif
        readcallback_();
    }
    if((revents_ & WriteEvent) && writecallback_)
    {
        #ifdef DEBUG
            std::cout<<__FUNCTION__<<" socket writable"<<std::endl;
        #endif
        writecallback_();
    }
}

int channel::send_bytes(const void* data, int len)
{
    std::cout<<len<<std::endl;
    int hasWritenToKernelBuffer = 0;
    int remainInOutputBuffer = 0;

    //如果写缓冲为空，则直接写内核缓冲区，此时内核缓冲区不满
    if(outputBuffer_.empty())
    {
        hasWritenToKernelBuffer =  write(fd, data, len);
        if(hasWritenToKernelBuffer > 0)
        {
           len -= hasWritenToKernelBuffer;

            #ifdef DEBUG
            std::cout<<"channel: "<<__LINE__<<" "<<hasWritenToKernelBuffer<<std::endl;
            #endif
        }
    }

    //此时需要写入缓冲区中，并且关注写事件
    if(len > 0)
    {
        readBuffer_.append((char*)data,len);
        enableWrite();
    }
}

void channel::update()
{
    base_loop->update_channel(shared_from_this());
}

//将内核缓冲区中的内容全部读到用户缓冲区中
void channel::handleRead()
{
    #ifdef DEBUG
        std::cout<<__FUNCTION__<<" "<<__LINE__<<std::endl;
    #endif

    char tempbuf[MAXBUFF];
    int readlen = readAllFromBuffer(fd, tempbuf, MAXBUFF);
    
    #ifdef DEBUG
        std::cout<<"channel: "<<__LINE__<<" "<<readlen<<std::endl;
    #endif

    if(readlen > 0 )
    {
        int len = readBuffer_.size();
        readBuffer_.append(tempbuf, readlen);

        //如果有回调函数，就调用该回调
        if(messagecallback_)
        {
            // auto threads = server->getThreadPool();
            messagecallback_(shared_from_this(), &readBuffer_[len], readlen);
        }
    }
    //如果可读，但是读出来的字节数为0，表示已经关闭了连接对方
    else if(readlen == 0)
    {
        disableAll();
    }
}


//内核缓冲区中有空闲的部分，将用户缓冲区中的内容写到内核缓冲区中
void channel::handleWrite()
{
    int WriteSize = outputBuffer_.size();
    int result = write(fd, outputBuffer_.c_str(), WriteSize);
    if(result > 0)
    {
        #ifdef DEBUG
            std::cout<<"channel: "<<__LINE__<<" "<<result<<std::endl;
        #endif

        WriteSize -=result;
    }
    if(WriteSize == 0 && writeCompleteCallBack_)
    {
        writeCompleteCallBack_();
    }
}

void channel::handleClose()
{
    disableAll();
    if(closecallback_)
        closecallback_();
}

ssize_t channel::readAllFromBuffer(int fd, char* data, size_t size)
{
    ssize_t len=0;
    len  = read(fd, data, size);

    if(len < 0 )
        printf("%s", strerror(errno));
    else if (len == 0)
        return len;

    int once = -1;
    while(once!= 0 &&len < size)
    {
        once = read(fd, data+len, size-len);
        if(once > 0)
            len += once;
        else   
            break;
    }

    #ifdef DEBUG
        printf("messege size: %ld, datalen: %ld\n", size, len);
    #endif

    return len;
}




