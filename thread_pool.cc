#include "thread_pool.hpp"
#include <iostream>

namespace jz
{
    ThreadPool::ThreadPool(eventloop* event_loop, int n)
        :loop(event_loop)
        ,tsg_(threads_)
        ,stop_(false)
    {
        //查看可以真正并发的线程数
        int nthreads = std::thread::hardware_concurrency();
        numthread = n<nthreads? n: nthreads;
        std::cout<<"number of the thread is " << numthread<<std::endl;
    }

    void ThreadPool::run()
    {
        
        auto self = shared_from_this();

        for(int i=0; i<numthread; ++i)
        {
            threads_.push_back(std::thread([self]
            {
                while (!self->stop_.load(std::memory_order_acquire))
                {
                    task task_;
                    {
                        std::unique_lock<std::mutex> ulk(self->mtx_);
                        self->cv_.wait(ulk, [self]{ return (self->stop_).load(std::memory_order_acquire) || !self->tasks_.empty(); });
                        if ((self->stop_).load(std::memory_order_acquire))
                            return;
                        task_ = std::move(self->tasks_.front());
                        self->tasks_.pop();
                    }
                    task_();
                }
            }
            ));
                //线程中运行的lambda函数
        }
    }

    ThreadPool::~ThreadPool()
    {
        stop();
        //唤醒所有等待任务的线程
        cv_.notify_all();
    }

    void ThreadPool::stop()
    {
        stop_.store(true, std::memory_order_release);
    } 
}