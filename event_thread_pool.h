#pragma once
#include<unordered_map>
#include<thread>
#include<mutex>
#include<functional>
#include<vector>
#include<condition_variable>
#include"util/queue.h"
namespace Event{
    const static int def_thread_number=3;
    class event_thread_pool{
        typedef std::function<void()> funType;
        int thread_number;
        std::queue<funType> task_list;
        std::mutex list_mutex;
        std::vector<std::thread> task_threads;
        std::condition_variable task_exec_cond;
        bool stop;
        public:
        event_thread_pool(int n = def_thread_number):thread_number(n),stop(false){
            if (n <= 0)
                throw std::exception();
            for (int i = 0; i < n; i++){
                task_threads.emplace_back([this](){//线程启动函数
                    while (!stop){//停止flag为真时停止线程
                        funType task;
                        std::unique_lock<std::mutex> lock(list_mutex);
                        while(task_list.empty() && !stop){//任务队列为空时等待唤醒
                            task_exec_cond.wait(lock);
                        }
                        if(!stop){//获取任务，停止时跳过
                            task=std::move(task_list.front());
                            task_list.pop();
                            lock.unlock();
                            task();
                        }
                    }
                });
            }
        }
        ~event_thread_pool(){
            stop = true;
            task_exec_cond.notify_all();
            for(auto &t : task_threads){
                t.join();
            }
        }
        int submit(funType task){
            std::unique_lock<std::mutex> lock(list_mutex);
            task_list.push(task);
            task_exec_cond.notify_one();
            return 0;
        }
    };
    std::unordered_map<std::string,event_thread_pool> event_thread_pools;
}