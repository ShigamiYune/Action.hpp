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

#include <vector>
#include <memory>

namespace action{
    struct key_callback {
        std::uintptr_t ptr;
        std::uintptr_t key;
    };

    template<typename SIGNATURE> class object_callback;
    template<typename RETURN, typename ... ARGS>
    class object_callback<RETURN(ARGS...)>{
    public:
        virtual ~object_callback(){}
        virtual RETURN invoke(ARGS ...) = 0;
        virtual bool compare(key_callback&) = 0;
    };

    namespace check {
        template <typename T>
        struct check_function {
        private:
            template <typename U>
            static constexpr bool is_lambda_no_capture() {
                return std::is_class_v<U> && std::is_convertible_v<U, void(*)()>;
            }

            template <typename U>
            static constexpr bool is_lambda_with_capture() {
                return std::is_class_v<U> && !is_lambda_no_capture<U>();
            }

        public:
            static constexpr unsigned char type =
                std::is_member_function_pointer_v<T> ? 0 :
                std::is_function_v<std::remove_pointer_t<T>> ? 1 :
                is_lambda_no_capture<T>() ? 3 :
                is_lambda_with_capture<T>() ? 4 : 255;
        };
    }

    namespace unpack {
        template<typename T>
        struct unpack;

        template<typename CLASS, typename RETURN, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...)> {
            using class_type = CLASS;
            using return_type = RETURN;
            using signature = RETURN(ARGS...);
        };

        template<typename CLASS, typename RETURN, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) const> {
            using class_type = CLASS;
            using return_type = RETURN;
            using signature = RETURN(ARGS...);
        };

        template<typename RETURN, typename... ARGS>
        struct unpack<RETURN(*)(ARGS...)> {
            using class_type = void;
            using return_type = RETURN;
            using signature = RETURN(ARGS...);
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) volatile> {
            using signature = RETURN(ARGS...);
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) const volatile> {
            using signature = RETURN(ARGS...);
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) &> { 
            using signature = RETURN(ARGS...); 
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) &&> { 
            using signature = RETURN(ARGS...); 
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) const &> { 
            using signature = RETURN(ARGS...); 
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) const &&> { 
            using signature = RETURN(ARGS...); 
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) noexcept> {
            using signature = RETURN(ARGS...); 
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) const noexcept> {
            using signature = RETURN(ARGS...); 
        };


        template<typename L>
        struct lambda_unpack {
            using pointer_type = decltype(&L::operator());
            using signature = typename unpack<pointer_type>::signature;
        };
    }

    namespace callback {
        template<auto FUNC, typename SIGNATURE, 
            unsigned char type = ::action::check::check_function<decltype(FUNC)>::type>
        struct callback;

        template<auto FUNC, typename RETURN, typename... ARGS>
        class callback<FUNC, RETURN(ARGS...), 0> : public object_callback<RETURN(ARGS...)>{
            using class_type = unpack::unpack<decltype(FUNC)>::class_type;
            class_type* _object;
        public:
            callback(class_type* object) : _object(object) {}

            RETURN invoke(ARGS... args) override{
                return (_object->*FUNC)(args...);
            }
            bool compare(key_callback& key) override {
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key.ptr) 
                    && 
                    (reinterpret_cast<std::uintptr_t>(FUNC) == key.key);
            }
        };

        template<auto FUNC, typename RETURN, typename... ARGS>
        class callback<FUNC, RETURN(ARGS...), 1> : public object_callback<RETURN(ARGS...)> {
        public:
            callback() = default;

            RETURN invoke(ARGS... args) override { return (*FUNC)(args...); }
            bool compare(key_callback& key) override {
                return (reinterpret_cast<std::uintptr_t>(FUNC) == key.key);
            }
        };

        template<auto FUNC, typename RETURN, typename... ARGS>
        class callback<FUNC, RETURN(ARGS...), 3> : public object_callback<RETURN(ARGS...)> {
        public:
            callback() = default;

            RETURN invoke(ARGS... args) override { return FUNC(args...); }
            bool compare(key_callback& key) override {
                return (reinterpret_cast<std::uintptr_t>(FUNC) == key.key);
            }
        };

        template<typename F, typename SIGNATURE, std::uintptr_t key>
        class lambda_callback;

        template<typename F, std::uintptr_t key , typename RETURN, typename... ARGS>
        class lambda_callback<F, RETURN(ARGS...), key> : public object_callback<RETURN(ARGS...)> {
            F func;
        public:
            explicit lambda_callback(F f) : func(std::move(f)) {}

            RETURN invoke(ARGS... args) override { return func(args...); }
            bool compare(key_callback& _key) override {
                return (key == _key.ptr) && (key == _key.key);
            }
        };
    }

    // ---- MAKE CALL BACK
    template<auto FUNC>
    auto make_callback(typename unpack::unpack<decltype(FUNC)>::class_type * object) {
        using sig = unpack::unpack<decltype(FUNC)>::signature;
        using cb_t = callback::callback<FUNC, sig>;
        return std::make_unique<cb_t>(object);
    }

    template<auto FUNC>
    auto make_callback() {
        using sig = unpack::unpack<decltype(FUNC)>::signature;
        using cb_t = callback::callback<FUNC, sig>;
        return std::make_unique<cb_t>();
    }

    template<std::uintptr_t key, typename FUNC>
    auto make_callback(FUNC f) {
        using sig = typename unpack::lambda_unpack<FUNC>::signature;
        using cb_t = ::action::callback::lambda_callback<FUNC, sig, key>;
        return std::make_unique<cb_t>(std::move(f));
    }

    // ---- GET KEY FOR CALL BACK
    template<typename CLASS, auto FUNC>
    key_callback get_key_callback(CLASS* object) {
        return key_callback(
            reinterpret_cast<std::uintptr_t>(object), 
            reinterpret_cast<std::uintptr_t>(FUNC));
    }

    template<auto FUNC>
    key_callback get_key_callback() {
        return key_callback(0, reinterpret_cast<std::uintptr_t>(FUNC));
    }

    template<std::uintptr_t key>
    key_callback get_key_callback() { return key_callback(key, key); }

    template<typename SIGNATURE> class action;
    template<typename RETURN, typename... ARGS> 
    class action<RETURN(ARGS...)> {
    public:
        using object_type = std::unique_ptr<object_callback<RETURN(ARGS...)>>;

        std::vector<object_type> callbacks;

        action* operator+=(object_type object) { 
            callbacks.push_back(std::move(object));  
            return this;
        }
        action* operator-=(key_callback& key) { 
            for (std::size_t i = 0; i < callbacks.size(); ++i) {
                if(callbacks[i]->compare(key)) {
                    callbacks.erase(callbacks.begin() + i);
                    return this;
                }
            }
            return this;
        }

        operator bool() { return !callbacks.empty(); }

        RETURN invoke(ARGS... args) {
            for (std::size_t i = 0; i < callbacks.size() - 1; ++i) {
                callbacks[i]->invoke(args...);
            }
            return callbacks[callbacks.size() - 1]->invoke(args...);
        }
    };
}

#endif
