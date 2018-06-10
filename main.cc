#include "tcpserver.hpp"
#include "channel.hpp"

using channelPtr = std::shared_ptr<channel>;

void addToThreadPool(channelPtr ch, char* buffer, int len)
{
    std::cout<<std::this_thread::get_id()<<std::endl;
    ch->send_bytes(buffer, len);      
}

int main(int argc, char** argv)
{
    eventloop base_loop; 
    tcpServer server(&base_loop, "127.0.0.1", 2333, 0, 4);

    server.setMessageCallBack(
            [&server](channelPtr ch, char* buffer, int len)
            {
                server.addToThreadPool(addToThreadPool, ch, buffer, len);
            }
    );

    server.start();

    return 0;
}