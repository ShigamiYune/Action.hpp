/*
 * Project Name: Action.hpp
 * Author: ShigamiYune
 * Copyright (c) 2025 ShigamiYune
 * License: MIT
 *
 * This file is licensed under the MIT License.
 * You must include this notice in any redistribution.
 */

#pragma once
#ifndef ACTION_H
#define ACTION_H

#pragma region ACTION_GEN

#define ACTION_COUNT_ARGS_IMPL(_1, _0, N, ...) N
#define ACTION_COUNT_ARGS(...) ACTION_COUNT_ARGS_IMPL(__VA_ARGS__, 1,0)

#define ACTION_CAT_IMPL(a,b) a##b
#define ACTION_CAT(a,b) ACTION_CAT_IMPL(a,b)

#define ACTION_GEN_PARAMS__0()
#define ACTION_GEN_PARAMS__1(T1) T1 a1

#define ACTION_GEN_PARAMS_1(...) ACTION_CAT(ACTION_GEN_PARAMS__, ACTION_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define ACTION_GEN_PARAMS_2(T1, T2)   T1 a1, T2 a2
#define ACTION_GEN_PARAMS_3(T1, T2, T3)   T1 a1, T2 a2, T3 a3
#define ACTION_GEN_PARAMS_4(T1, T2, T3, T4)   T1 a1, T2 a2, T3 a3, T4 a4
#define ACTION_GEN_PARAMS_5(T1, T2, T3, T4, T5)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5
#define ACTION_GEN_PARAMS_6(T1, T2, T3, T4, T5, T6)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6
#define ACTION_GEN_PARAMS_7(T1, T2, T3, T4, T5, T6, T7)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7
#define ACTION_GEN_PARAMS_8(T1, T2, T3, T4, T5, T6, T7, T8)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8
#define ACTION_GEN_PARAMS_9(T1, T2, T3, T4, T5, T6, T7, T8, T9)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9
#define ACTION_GEN_PARAMS_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9, T10 a10
#define ACTION_GEN_PARAMS_11(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9, T10 a10, T11 a11
#define ACTION_GEN_PARAMS_12(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9, T10 a10, T11 a11, T12 a12
#define ACTION_GEN_PARAMS_13(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9, T10 a10, T11 a11, T12 a12, T13 a13
#define ACTION_GEN_PARAMS_14(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9, T10 a10, T11 a11, T12 a12, T13 a13, T14 a14
#define ACTION_GEN_PARAMS_15(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9, T10 a10, T11 a11, T12 a12, T13 a13, T14 a14, T15 a15
#define ACTION_GEN_PARAMS_16(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9, T10 a10, T11 a11, T12 a12, T13 a13, T14 a14, T15 a15, T16 a16
#define ACTION_GEN_PARAMS_17(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9, T10 a10, T11 a11, T12 a12, T13 a13, T14 a14, T15 a15, T16 a16, T17 a17
#define ACTION_GEN_PARAMS_18(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9, T10 a10, T11 a11, T12 a12, T13 a13, T14 a14, T15 a15, T16 a16, T17 a17, T18 a18
#define ACTION_GEN_PARAMS_19(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9, T10 a10, T11 a11, T12 a12, T13 a13, T14 a14, T15 a15, T16 a16, T17 a17, T18 a18, T19 a19
#define ACTION_GEN_PARAMS_20(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20)   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9, T10 a10, T11 a11, T12 a12, T13 a13, T14 a14, T15 a15, T16 a16, T17 a17, T18 a18, T19 a19, T20 a20

#define ACTION_GEN_VAR__0()
#define ACTION_GEN_VAR__1(T1) a1

#define ACTION_GEN_VAR_1(...) ACTION_CAT(ACTION_GEN_VAR__, ACTION_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)
#define ACTION_GEN_VAR_2(T1, T2)   a1, a2
#define ACTION_GEN_VAR_3(T1, T2, T3)   a1, a2, a3
#define ACTION_GEN_VAR_4(T1, T2, T3, T4)   a1, a2, a3, a4
#define ACTION_GEN_VAR_5(T1, T2, T3, T4, T5)   a1, a2, a3, a4, a5
#define ACTION_GEN_VAR_6(T1, T2, T3, T4, T5, T6)   a1, a2, a3, a4, a5, a6
#define ACTION_GEN_VAR_7(T1, T2, T3, T4, T5, T6, T7)   a1, a2, a3, a4, a5, a6, a7
#define ACTION_GEN_VAR_8(T1, T2, T3, T4, T5, T6, T7, T8)   a1, a2, a3, a4, a5, a6, a7, a8
#define ACTION_GEN_VAR_9(T1, T2, T3, T4, T5, T6, T7, T8, T9)   a1, a2, a3, a4, a5, a6, a7, a8, a9
#define ACTION_GEN_VAR_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10)   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10
#define ACTION_GEN_VAR_11(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11)   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11
#define ACTION_GEN_VAR_12(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12)   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12
#define ACTION_GEN_VAR_13(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13)   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13
#define ACTION_GEN_VAR_14(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14)   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14
#define ACTION_GEN_VAR_15(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15)   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15
#define ACTION_GEN_VAR_16(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16)   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16
#define ACTION_GEN_VAR_17(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17)   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17
#define ACTION_GEN_VAR_18(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18)   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18
#define ACTION_GEN_VAR_19(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19)   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19
#define ACTION_GEN_VAR_20(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20)   a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20

#define ACTION_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,NAME, ...) NAME

#define ACTION_GEN_PARAMS(...) \
    ACTION_GET_MACRO(__VA_ARGS__, ACTION_GEN_PARAMS_20, ACTION_GEN_PARAMS_19, ACTION_GEN_PARAMS_18, ACTION_GEN_PARAMS_17, ACTION_GEN_PARAMS_16, ACTION_GEN_PARAMS_15, ACTION_GEN_PARAMS_14, ACTION_GEN_PARAMS_13, ACTION_GEN_PARAMS_12, ACTION_GEN_PARAMS_11, ACTION_GEN_PARAMS_10, ACTION_GEN_PARAMS_9, ACTION_GEN_PARAMS_8, ACTION_GEN_PARAMS_7, ACTION_GEN_PARAMS_6, ACTION_GEN_PARAMS_5, ACTION_GEN_PARAMS_4, ACTION_GEN_PARAMS_3, ACTION_GEN_PARAMS_2, ACTION_GEN_PARAMS_1)(__VA_ARGS__)

#define ACTION_GEN_VAR(...) \
    ACTION_GET_MACRO(__VA_ARGS__, ACTION_GEN_VAR_20, ACTION_GEN_VAR_19, ACTION_GEN_VAR_18, ACTION_GEN_VAR_17, ACTION_GEN_VAR_16, ACTION_GEN_VAR_15, ACTION_GEN_VAR_14, ACTION_GEN_VAR_13, ACTION_GEN_VAR_12, ACTION_GEN_VAR_11, ACTION_GEN_VAR_10, ACTION_GEN_VAR_9, ACTION_GEN_VAR_8, ACTION_GEN_VAR_7, ACTION_GEN_VAR_6, ACTION_GEN_VAR_5, ACTION_GEN_VAR_4, ACTION_GEN_VAR_3, ACTION_GEN_VAR_2, ACTION_GEN_VAR_1)(__VA_ARGS__)

#define ACTION_SIG_TO_PARAMS_I(...) __VA_ARGS__
#define ACTION_SIG_TO_PARAMS(X) ACTION_SIG_TO_PARAMS_I X

#pragma endregion

#include <vector>
#include <memory>

struct KeyCallback{
    const void* ptr;
    const wchar_t* name;
};

template<typename Signature> class ObjectCallback;
template<typename Return, typename ... Args>
class ObjectCallback<Return(Args...)>{
public:
    virtual ~ObjectCallback(){}
    virtual Return invoke(Args ...) = 0;
    virtual bool compare(const KeyCallback& key) const {return false;}
};

template<typename Signature> class Action;
template<typename Return, typename ... Args>
class Action<Return(Args...)> {
    std::vector<std::unique_ptr<ObjectCallback<Return(Args ...)>>> callback;
public:
    void add_callback(std::unique_ptr<ObjectCallback<Return(Args ...)>> _callback){
        callback.push_back(std::move(_callback));
    }

    Return invoke(Args ... arg){
        std::size_t size = callback.size();
        if(size == 0) return {}; 
        size -= 1;
        for(int i = 0; i < size; i++){
            callback[i]->invoke(arg...);
        }
        return callback[size]->invoke(arg...);
    }

    Action& operator+=(std::unique_ptr<ObjectCallback<Return(Args ...)>> _callback) {
        callback.push_back(std::move(_callback));
        return *this;
    }

    Action& operator-=(const KeyCallback& key) {
        auto it = std::remove_if(callback.begin(), callback.end(),
            [key](const std::unique_ptr<ObjectCallback<Return(Args ...)>>& e) {
                return e.get()->compare(key);
            });

        callback.erase(it, callback.end());
        return *this;
    }

    std::size_t size() const { return callback.size(); }
    void clear() { callback.clear(); }

    Return invoke_with_key(const KeyCallback& key, Args ... arg){
        std::size_t size = callback.size();
        if(size == 0) return {}; 
        for(int i = 0; i < size; i++){
            if(callback[i].get()->compare(key)) return callback[i].get()->invoke(arg ...);
        }
        return {}; 
    }
};

template<typename ... Args>
class Action<void(Args...)> {
    std::vector<std::unique_ptr<ObjectCallback<void(Args ...)>>> callback;
public:
    void add_callback(std::unique_ptr<ObjectCallback<void(Args ...)>> _callback){
        callback.push_back(std::move(_callback));
    }

    void invoke(Args ... arg){
        std::size_t size = callback.size();
        if(size == 0) return; 
        size -= 1;
        for(int i = 0; i < size; i++){
            callback[i]->invoke(arg...);
        }
        callback[size]->invoke(arg...);
    }

    Action& operator+=(std::unique_ptr<ObjectCallback<void(Args ...)>> _callback) {
        callback.push_back(std::move(_callback));
        return *this;
    }

    Action& operator-=(const KeyCallback& key) {
        auto it = std::remove_if(callback.begin(), callback.end(),
            [key](const std::unique_ptr<ObjectCallback<void(Args ...)>>& e) {
                return e.get()->compare(key);
            });

        callback.erase(it, callback.end());
        return *this;
    }

    std::size_t size() const { return callback.size(); }
    void clear() { callback.clear(); }

    void invoke_with_key(const KeyCallback& key, Args ... arg){
        std::size_t size = callback.size();
        if(size == 0) return; 
        for(int i = 0; i < size; i++){
            if(callback[i].get()->compare(key)) callback[i].get()->invoke(arg ...);
        }
    }
};

template<typename T, typename Signature> class Lambda_Local;
template<typename T, typename Return, typename ... Args>
class Lambda_Local<T, Return(Args ...)> : public ObjectCallback<Return(Args ...)> {
    public:
    T fn; Lambda_Local(T _fn) : fn(_fn) {}
    Return invoke(Args ... args) override { return fn(args ...);}
};

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)

#define CALLBACK_MEMBER(CLASS, FUNC, PTR, RET, SIG)                                                                     \
[PTR] () {                                                                                                              \
    struct CallBack : ObjectCallback<RET(SIG)>{                                                                         \
        CLASS* ptr = nullptr; CallBack(CLASS* _ptr) : ptr(_ptr){}                                                       \
        RET invoke(ACTION_GEN_PARAMS(ACTION_SIG_TO_PARAMS(SIG))) override {                                             \
             return ptr->FUNC(ACTION_GEN_VAR(ACTION_SIG_TO_PARAMS(SIG)));                                               \
        }                                                                                                               \
        bool compare(const KeyCallback& key) const override {                                                           \
             return ptr == key.ptr && wcscmp(key.name, WIDEN(#FUNC)) == 0;                                              \
        }                                                                                                               \
    };                                                                                                                  \
    return std::make_unique<CallBack>(PTR);                                                                             \
}()

#define CALLBACK_STATIC_MEMBER(CLASS, FUNC, CALL, RET, SIG)                                                             \
[] () {                                                                                                                 \
    struct Callback : ObjectCallback<RET(SIG)>{                                                                         \
        RET invoke(ACTION_GEN_PARAMS(ACTION_SIG_TO_PARAMS(SIG))) override {                                             \
             return CALL(ACTION_GEN_VAR(ACTION_SIG_TO_PARAMS(SIG)));                                                    \
        }                                                                                                               \
        bool compare(const KeyCallback& key) const override {                                                           \
             return &CALL == key.ptr && wcscmp(key.name, WIDEN(#FUNC)) == 0;                                            \
        }                                                                                                               \
    };                                                                                                                  \
    return std::make_unique<Callback>();                                                                                \
}()

#define CALLBACK_GLOBAL(FUNC, CALL, RET, SIG)                                                                           \
[] () {                                                                                                                 \
    struct Callback : ObjectCallback<RET(SIG)>{                                                                         \
        RET invoke(ACTION_GEN_PARAMS(ACTION_SIG_TO_PARAMS(SIG))) override {                                             \
             return CALL(ACTION_GEN_VAR(ACTION_SIG_TO_PARAMS(SIG)));                                                    \
        }                                                                                                               \
        bool compare(const KeyCallback& key) const override {                                                           \
             return reinterpret_cast<const void*>(                                                                      \
                reinterpret_cast<uintptr_t>(&CALL)) == key.ptr && wcscmp(key.name, WIDEN(#FUNC)) == 0;                  \
        }                                                                                                               \
    };                                                                                                                  \
    return std::make_unique<Callback>();                                                                                \
}()

#define CALLBACK_LAMBDA_LOCAL(FUNC, RET, SIG)                                                                           \
[FUNC] () {                                                                                                             \
    struct Callback : Lambda_Local<decltype(FUNC), RET(SIG)>{                                                           \
        Callback(decltype(FUNC) _fn) : Lambda_Local(_fn){}                                                              \
        RET invoke(ACTION_GEN_PARAMS(ACTION_SIG_TO_PARAMS(SIG))) override {                                             \
             return fn(ACTION_GEN_VAR(ACTION_SIG_TO_PARAMS(SIG)));                                                      \
        }                                                                                                               \
        bool compare(const KeyCallback& key) const override {                                                           \
             return wcscmp(key.name, WIDEN(#FUNC)) == 0;                                                                \
        }                                                                                                               \
    };                                                                                                                  \
    return std::make_unique<Callback>(FUNC);                                                                            \
}()

// Key for CALLBACK_LAMBDA_LOCAL
#define GET_KEY_CALLBACK_CALLABLE(FUNC) KeyCallback(nullptr, WIDEN(#FUNC))
// Key for CALLBACK_MEMBER
#define GET_KEY_CALLBACK_MEMBER(CLASS, FUNC, PTR) KeyCallback(PTR, WIDEN(#FUNC))
// Key for CALLBACK_MEMBER_STATIC
#define GET_KEY_CALLBACK_STATIC(CLASS, FUNC, CALL) KeyCallback(&CALL, WIDEN(#FUNC))
// Key for CALLBACK_GLOBAL
#define GET_KEY_CALLBACK_GLOBAL(FUNC, CALL) KeyCallback(\
    reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(&CALL)), WIDEN(#FUNC))
#endif
