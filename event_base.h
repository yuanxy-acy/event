#pragma once
#include<string>
namespace Event{
    class event_base
    {
    public:
        event_base(){}
        ~event_base(){}
        virtual std::string get_name() = 0;
    };
}