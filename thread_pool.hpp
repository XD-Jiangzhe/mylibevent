#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include "eventloop.hpp"

#include <vector>
#include <thread>
#include <future>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <utility>
#include <functional>
#include <mutex>
#include <memory>

using std::vector;
using std::packaged_task;
namespace jz
{
    //这里做了一个线程池的守卫
    class threadsGuard
    {
        public:
            threadsGuard(std::vector<std::thread>& v):threads_(v){}
            ~threadsGuard()
            {
                for(size_t i=0; i!= threads_.size(); ++i)
                {
                    if(threads_[i].joinable())
                    {
                        threads_[i].join();
                    }
                }
            }

        private:
            //将拷贝构造,赋值构造,移动构造 删除
            threadsGuard(threadsGuard&& tg) = delete;
            threadsGuard& operator=(threadsGuard&& tg) = delete;
            
            threadsGuard(const threadsGuard&) =delete;
            threadsGuard& operator=(threadsGuard& tg) = delete;

        private:
            std::vector<std::thread>& threads_;
    };

    class ThreadPool :public std::enable_shared_from_this<ThreadPool>
    {
        public:
            typedef std::function<void()> task;

        public:
            explicit ThreadPool(eventloop* ,int n=0);
            ~ThreadPool();
            
            //暂停线程池
            void stop();
            void run();
            //往队列中加任务,任务的返回是一个future
            //可以在之后的变量中取到
        template<class Function, class... Args>
        std::future<typename std::result_of<Function(Args...)>::type> 
        add(Function&& fcn, Args&&... args);
            
        //后面变成多线程中的改动,增加该线程池所述的io线程
        private:
            eventloop* loop;

        private:
            //每个线程从queue中取出任务
            task take();

        private:
            int numthread;
            //线程的个数
            std::atomic<bool> stop_;
            //使用互斥锁来保护条件变量
            std::mutex mtx_;
            std::condition_variable cv_;

            //
            std::queue<task> tasks_;
            std::vector<std::thread> threads_;
            threadsGuard tsg_;
    };

    //模板函数的定义和实现要放在一个hpp文件中,跟inline一样,否则会找不到定义
    template<typename Function, typename... Args>
    std::future<typename std::result_of<Function(Args...)>::type>
    ThreadPool::add(Function&& func, Args&&...args)
    {
        using return_type = typename std::result_of<Function(Args...)>::type;
        using package_func =  std::packaged_task<return_type()>;

        //因为参数都给了,所以package_task封装的函数没有占位符的参数
        //所以package_task的类型是 return_type()
        auto t = std::make_shared<package_func>(
                std::bind(std::forward<Function>(func), std::forward<Args>(args)...));
        //通过make_shared构造了一个package_task的智能指针
        //
        auto result = t->get_future();
        {
            std::lock_guard<std::mutex> lg(mtx_);
            if(stop_.load(std::memory_order_acquire))
                throw std::runtime_error("thread pool has stopped");
            tasks_.emplace([t]{(*t)();});
        //这个t是一个shared_ptr,用来指向一个func,并返回一个future
        //装入了一个packaged_task
        //这里用lambda封装了一个函数对象,void(),然后将该t的结果绑在了一个future上
        //所以这里可以放入任意的函数,而不用顾忌返回值的问题
        }

        cv_.notify_one();
        return result;
    }

}

#endif
