#pragma once
#ifndef ACTION_H
#define ACTION_H

#include <vector>
#include <memory>

template<typename Signature>
class ObjectEvent;

template<typename Return, typename ... Args>
class ObjectEvent<Return(Args...)>{
public:
    virtual ~ObjectEvent(){}
    virtual Return invoke(Args ...) = 0;
};

template<typename Signature>
class Action;

template<typename Return, typename ... Args>
class Action<Return(Args...)> {
    std::vector<std::unique_ptr<ObjectEvent<Return(Args ...)>>> event;
public:
    void add_event(std::unique_ptr<ObjectEvent<Return(Args ...)>> _event){
        event.push_back(std::move(_event));
    }

    Return invoke(Args ... arg){
        std::size_t size = event.size() - 1;
        for(int i = 0; i < size; i++){
            event[i]->invoke(arg...);
        }
        return event[size]->invoke(arg...);
    }

    Action& operator+=(std::unique_ptr<ObjectEvent<Return(Args ...)>> _event) {
        event.push_back(std::move(_event));
        return *this;
    }

    Action& operator-=(ObjectEvent<Return(Args ...)>* target_ptr) {
        auto it = std::remove_if(event.begin(), event.end(),
            [target_ptr](const std::unique_ptr<ObjectEvent<Return(Args ...)>>& e) {
                return e.get() == target_ptr;
            });

        event.erase(it, event.end());
        return *this;
    }
};

#define MAKE_EVENT(FUNC, CLASS, Signature) std::make_unique<EVENT##FUNC##CLASS<Signature>>(this);
#define DEFINE_EVENT(FUNC, CLASS)                                               \
    template<typename Signature> struct EVENT##FUNC##CLASS;                     \
    template<typename Return, typename ... Args>                                \
    struct EVENT##FUNC##CLASS<Return(Args ...)> : ObjectEvent<Return(Args ...)>{\
        CLASS* ptr = nullptr; EVENT##FUNC##CLASS(CLASS* _ptr) : ptr(_ptr){}     \
        Return invoke(Args ... args) override { return ptr->FUNC(args ...); }   \
    }; 

#endif
