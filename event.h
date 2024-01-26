#pragma once
#include<map>
#include"event_thread_pool.h"
#include"event_base.h"
namespace Event{
    template<typename T> class event;
    template<typename returnType,typename... ArgTypes>
    class event<returnType(ArgTypes...)>:public event_base{
        protected:
        std::function<returnType(ArgTypes...)> callback;
        const std::string event_name;
        util::queue<std::function<returnType()>> exec_que;
        util::queue<returnType> result_que;
        public:
        typedef std::function<returnType(ArgTypes...)> callbackType;
        event():event_name("event"){}
        event(callbackType cb):event_name("event"),callback(cb){}
        event(std::string n):event_name(n){}
        event(std::string n,callbackType cb):event_name(n),callback(cb){}
        virtual returnType exec(ArgTypes... args){//调用回调函数
            return callback(args...);
        }
        virtual int push(ArgTypes... args){
            exec_que.push(std::function([=](){//封装参数列表
                return callback(args...);
            }));
            return 0;
        }
        virtual int pop(){
            std::unique_lock<std::mutex> lock(exec_que.mux);
            std::function<returnType()> _exe = exec_que.data.front();
            exec_que.data.pop();
            lock.unlock();
            event_thread_pools[event_name].submit([this,_exe](){//向同名事件共享线程池提交任务
                returnType res = _exe();
                this->result_que.push(std::move(res));//执行结果推入结果队列
            });
            return 0;
        }
        virtual std::string get_name(){
            return event_name;
        }
        void set_callback(callbackType cb){
            callback = cb;
        }
        returnType get_result(){//获取结果
            bool emp=true;
            while (emp){//当结果队列为空时等待
                emp=result_que.empty();
            }
            std::unique_lock<std::mutex> lock(result_que.mux);
            returnType ret = result_que.data.front();
            result_que.data.pop();
            return ret;
        }
        void run_all(){//取当前所有执行函数丢入线程池
            std::unique_lock<std::mutex> lock(exec_que.mux);
            while (!exec_que.data.empty())
            {
                std::function<returnType()> _exe = exec_que.data.front();
                exec_que.data.pop();
                event_thread_pools[event_name].submit([this,_exe](){//向同名事件共享线程池提交任务
                    returnType res = _exe();
                    this->result_que.push(std::move(res));//执行结果推入结果队列
                });
            }
        }
    };
}