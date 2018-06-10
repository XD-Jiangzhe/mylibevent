# mylibevent
自己实现的基于c++11的网络库 

## 使用方法  
cmake .  
cmake version >=3.5  
接口简单，只要绑定messageCallBack，并且有channel类表示两端的连接类，可以send buffer即可  
然后将callback add进tcpserver自带的线程池即可 ，如果不add就在eventloop线程中执行   

## 开发过程  
使用了c++11中的并发的API，lambda以及新加入的模板变参  
仿照muduo和libevent的设计思路写的  
下一步加入定时器和将buffer类丰富  

buffer类是用string粗劣代替的，下一步是实现buffer类，使用链表或是 栈上大数组来代替  
定时器不打算使用timerfd 系统自带的定时套接字，打算仿照libevent使用小根堆的形式来实现  
上面的这两步等有空再弄  

接下来是支持多线程部分，即单个listener 然后将做好的连接放入其他的eventloop中
将连接和IO彻底进行分离，目前是连接和IO都是在主event_loop线程中的  
