#include "listener.hpp"

int listenAndBind(sockaddr_in sock, int listenSize)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    int enable =1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    if(fd < 0)
        printf("socket error\n");

    if(bind(fd, (sockaddr*)&sock, sizeof(sockaddr)) < 0)
        printf("bind error\n");
    

    listen(fd, listenSize);
    
    return fd;
}

listener::listener(eventloop* base_loop, sockaddr_in sock, int listensize)
                :Loop(base_loop)
                ,sock_(sock)
                ,listenChannel(std::make_shared<channel>(Loop, listenAndBind(sock, listensize)))
{
    listenInit();
}

listener::listener(eventloop* base_loop, std::string IpAddr, int port, int listenSize)
                :Loop(base_loop)
                ,sock_({AF_INET, htons(port), inet_addr(IpAddr.data())})
                ,listenChannel(std::make_shared<channel>(Loop, listenAndBind(sock_, listenSize)))                
{
    listenInit();
}

void listener::listenInit()
{
    listenChannel->enableRead();
    listenChannel->setReadCallBack(std::bind(&listener::handle_read, this));
}

void listener::handle_read()
{
    socklen_t addrlen = sizeof(sockaddr);
    int connfd = accept(listenChannel->getfd(), (sockaddr*)&sock_, &addrlen);
    if(connfd < 0)
    {
        std::cout<<"listener: "<<__FILE__<<" accept fail"<<std::endl;
    }
    else 
    {
        //设置成非阻塞套接字
        int flag = fcntl(connfd, F_GETFL, 0);
        fcntl(connfd, F_SETFL, flag|O_NONBLOCK);
        
        #ifdef DEBUG
        std::cout<<"connfd: "<<__LINE__<<" "<<connfd<<std::endl;
        #endif
    }
    if(connfd > 0 && newConnectionCallBack)
        newConnectionCallBack(Loop, connfd);
    
}
