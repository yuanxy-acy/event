#pragma once
#include<mutex>
#include<queue>
namespace Event::util{
    template<typename T>
    struct queue
    {
        std::queue<T> data;
        std::mutex mux;
        void push(T&& in){
            std::lock_guard<std::mutex> lock(mux);
            data.push(in);
        }
        void pop(){
            std::lock_guard<std::mutex> lock(mux);
            data.pop();
        }
        T& front(){
            std::lock_guard<std::mutex> lock(mux);
            return data.front();
        }
        bool empty(){
            std::lock_guard<std::mutex> lock(mux);
            return data.empty();
        }
        T& front_pop(){
            std::lock_guard<std::mutex> lock(mux);
            T out = data.front();
            data.pop();
            return out;
        }
    };
}