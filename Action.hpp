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
#include <typeindex>

namespace action{
    namespace check {
        template <typename T>
        struct check_function {
        private:
            template <typename U>
            struct is_lambda_no_capture_helper {
                static const bool value =
                    std::is_class<U>::value &&
                    std::is_convertible<U, void(*)()>::value;
            };

            template <typename U>
            struct is_lambda_with_capture_helper {
                static const bool value =
                    std::is_class<U>::value &&
                    !is_lambda_no_capture_helper<U>::value;
            };

            // helper detect const member function
            template <typename U>
            struct is_const_member_function_pointer : std::false_type {};

            template <typename C, typename R, typename... Args>
            struct is_const_member_function_pointer<R(C::*)(Args...) const> : std::true_type {};

        public:
            static const unsigned char type =
                std::is_member_function_pointer<T>::value ?
                    (is_const_member_function_pointer<T>::value ? 2 : 0) :
                std::is_function<typename std::remove_pointer<T>::type>::value ? 1 :
                is_lambda_no_capture_helper<T>::value ? 3 :
                is_lambda_with_capture_helper<T>::value ? 4 : 255;
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
            using func_type = RETURN(CLASS::*)(ARGS...);
        };

        template<typename CLASS, typename RETURN, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) const> {
            using class_type = CLASS;
            using return_type = RETURN;
            using signature = RETURN(ARGS...);
            using func_type = RETURN(CLASS::*)(ARGS...) const;
        };

        template<typename RETURN, typename... ARGS>
        struct unpack<RETURN(*)(ARGS...)> {
            using class_type = void;
            using return_type = RETURN;
            using signature = RETURN(ARGS...);
            using func_type = RETURN(*)(ARGS...);
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) volatile> {
            using class_type = CLASS;
            using return_type = RETURN;
            using signature = RETURN(ARGS...);
            using func_type = RETURN(CLASS::*)(ARGS...) volatile;
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) const volatile> {
            using class_type = CLASS;
            using return_type = RETURN;
            using signature = RETURN(ARGS...);
            using func_type = RETURN(CLASS::*)(ARGS...) const volatile;
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) &> { 
            using class_type = CLASS;
            using return_type = RETURN;
            using signature = RETURN(ARGS...); 
            using func_type = RETURN(CLASS::*)(ARGS...) &;
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) &&> { 
            using class_type = CLASS;
            using return_type = RETURN;
            using signature = RETURN(ARGS...); 
            using func_type = RETURN(CLASS::*)(ARGS...) &&;
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) const &> { 
            using class_type = CLASS;
            using return_type = RETURN;
            using signature = RETURN(ARGS...);
            using func_type = RETURN(CLASS::*)(ARGS...) const &;
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) const &&> { 
            using class_type = CLASS;
            using return_type = RETURN;
            using signature = RETURN(ARGS...); 
            using func_type = RETURN(CLASS::*)(ARGS...) const &&;
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) noexcept> {
            using class_type = CLASS;
            using return_type = RETURN;
            using signature = RETURN(ARGS...); 
            using func_type = RETURN(CLASS::*)(ARGS...) noexcept ;
        };

        template<typename RETURN, typename CLASS, typename... ARGS>
        struct unpack<RETURN(CLASS::*)(ARGS...) const noexcept> {
            using class_type = CLASS; 
            using return_type = RETURN;
            using signature = RETURN(ARGS...); 
            using func_type = RETURN(CLASS::*)(ARGS...) const noexcept;
        };


        template<typename L>
        struct lambda_unpack {
            using pointer_type = decltype(&L::operator());
            using signature = typename unpack<pointer_type>::signature;
        };
    }

    namespace callback {
        class key_callback_member {
        public:
            std::uintptr_t ptr;
            std::type_index func_type;

            key_callback_member(std::uintptr_t obj_ptr, std::type_index type)
                : ptr(obj_ptr), func_type(type) {}
        };

        template<typename FUNC_T>
        struct d_key_callback_member : key_callback_member {
            FUNC_T func_ptr;

            d_key_callback_member(std::uintptr_t obj_ptr, FUNC_T fptr)
                : key_callback_member(obj_ptr, std::type_index(typeid(FUNC_T))),
                func_ptr(fptr) {}
        };

        template<typename SIGNATURE> struct key_callback_global;
        template<typename RETURN, typename... ARGS>
        struct key_callback_global<RETURN(ARGS...)> {
            RETURN(*ptr)(ARGS...);
            key_callback_global(RETURN(*func)(ARGS...)) : ptr(func) {}
        };

        struct key_callback_lambda { 
            std::uintptr_t key;
            key_callback_lambda(std::uintptr_t _key) : key(_key) {}
        };

        template<typename SIGNATURE> class object_callback;
        template<typename RETURN, typename ... ARGS>
        class object_callback<RETURN(ARGS...)>{
        public:
            virtual ~object_callback(){}
            virtual RETURN invoke(ARGS ...) = 0;
            virtual bool compare(const key_callback_member*) = 0;
            virtual bool compare(const key_callback_global<RETURN(ARGS...)>&) = 0;
            virtual bool compare(const key_callback_lambda&) = 0;
        };

        template<auto FUNC, typename SIGNATURE, 
            unsigned char type = ::action::check::check_function<decltype(FUNC)>::type>
        struct callback;

        template<auto FUNC, typename RETURN, typename... ARGS>
        class callback<FUNC, RETURN(ARGS...), 0> : public object_callback<RETURN(ARGS...)>{
            using class_type = typename unpack::unpack<decltype(FUNC)>::class_type;
            using signature = typename unpack::unpack<decltype(FUNC)>::signature;
            using func_type = typename unpack::unpack<decltype(FUNC)>::func_type;

            class_type* _object;
        public:
            callback(class_type* object) : _object(object) {}

            RETURN invoke(ARGS... args) override{
                return (_object->*FUNC)(args...);
            }
            bool compare(const key_callback_member* key) override {
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(func_type)) == key->func_type)
                    &&
                    (static_cast<const d_key_callback_member<func_type>*>(key)->func_ptr == FUNC)
                    ;
            }
            bool compare(const key_callback_global<RETURN(ARGS...)>& key) override { return false; }
            bool compare(const key_callback_lambda& key) override { return false; }
        };

        template<auto FUNC, typename RETURN, typename... ARGS>
        class callback<FUNC, RETURN(ARGS...), 2> : public object_callback<RETURN(ARGS...)>{
            using class_type = typename unpack::unpack<decltype(FUNC)>::class_type;
            using signature = typename unpack::unpack<decltype(FUNC)>::signature;
            using func_type = typename unpack::unpack<decltype(FUNC)>::func_type;

            const class_type* _object;
        public:
            callback(const class_type* object) : _object(object) {}

            RETURN invoke(ARGS... args) override{
                return (_object->*FUNC)(args...);
            }
            bool compare(const key_callback_member* key) override {
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(func_type)) == key->func_type)
                    &&
                    (static_cast<const d_key_callback_member<func_type>*>(key)->func_ptr == FUNC)
                    ;
            }
            bool compare(const key_callback_global<RETURN(ARGS...)>& key) override { return false; }
            bool compare(const key_callback_lambda& key) override { return false; }
        };

        template<auto FUNC, typename RETURN, typename... ARGS>
        class callback<FUNC, RETURN(ARGS...), 1> : public object_callback<RETURN(ARGS...)> {
        public:
            callback() = default;

            RETURN invoke(ARGS... args) override { return (*FUNC)(args...); }
            bool compare(const key_callback_member* key) override { return false; }
            bool compare(const key_callback_global<RETURN(ARGS...)>& key) override { 
                return key.ptr == FUNC; 
            }
            bool compare(const key_callback_lambda& key) override { return false; }
        };

        template<auto FUNC, typename RETURN, typename... ARGS>
        class callback<FUNC, RETURN(ARGS...), 3> : public object_callback<RETURN(ARGS...)> {
        public:
            callback() = default;

            RETURN invoke(ARGS... args) override { return FUNC(args...); }
            bool compare(const key_callback_member& key) override { return false; }
            bool compare(const key_callback_global<RETURN(ARGS...)>& key) override { 
                return key.ptr == FUNC; 
            }
            bool compare(const key_callback_lambda& key) override { return false; }
        };

        template<typename F, typename SIGNATURE, std::uintptr_t key>
        class lambda_callback;

        template<typename F, std::uintptr_t key , typename RETURN, typename... ARGS>
        class lambda_callback<F, RETURN(ARGS...), key> : public object_callback<RETURN(ARGS...)> {
            F func;
        public:
            explicit lambda_callback(F f) : func(std::move(f)) {}

            RETURN invoke(ARGS... args) override { return func(args...); }
            bool compare(const key_callback_member* _key) override { return false; }
            bool compare(const key_callback_global<RETURN(ARGS...)>& _key) override { return false; }
            bool compare(const key_callback_lambda& _key) override { 
                return _key.key == key;
            }
        };
    }
    template<auto FUNC, typename WRAP, 
        typename = std::enable_if_t<check::check_function<decltype(FUNC)>::type == 0>>
    auto make_callback(WRAP object) {
        using sig = unpack::unpack<decltype(FUNC)>::signature;
        using cb_t = callback::callback<FUNC, sig>;
        return std::make_unique<cb_t>(object);
    }

    template<typename SIGNATURE> class action;
    template<typename RETURN, typename... ARGS> 
    class action<RETURN(ARGS...)> {
    public:
        using object_type = std::unique_ptr<callback::object_callback<RETURN(ARGS...)>>;

        std::vector<object_type> callbacks;

        operator bool() { return !callbacks.empty(); }

        RETURN invoke(ARGS... args) {
            if(callbacks.empty()) return {};
            for (std::size_t i = 0; i < callbacks.size() - 1; ++i) {
                callbacks[i]->invoke(args...);
            }
            return callbacks[callbacks.size() - 1]->invoke(args...);
        }

        // member
        template<auto FUNC, typename T>
        void push_back(T* object) {
            using cb_t = callback::callback<FUNC, RETURN(ARGS...)>;
            callbacks.push_back(std::make_unique<cb_t>(object));
        }
        // global
        template<auto FUNC>
        void push_back() {
            using cb_t = callback::callback<FUNC, RETURN(ARGS...)>;
            callbacks.push_back(std::make_unique<cb_t>());
        }
        // lambda
        template<std::uintptr_t key, typename FUNC>
        void push_back(FUNC f) {
            using cb_t = callback::lambda_callback<FUNC, RETURN(ARGS...), key>;
            callbacks.push_back(std::make_unique<cb_t>(std::move(f)));
        }

        // member
        template <typename T>
        void erase(RETURN (T::*func)(ARGS...), const T* object) {
            auto key = callback::d_key_callback_member<decltype(func)>(
                reinterpret_cast<std::uintptr_t>(object),
                func
            );
            for (std::size_t i = 0; i < callbacks.size(); ++i) {
                if(callbacks[i]->compare(&key)) {
                    callbacks.erase(callbacks.begin() + i);
                    return;
                }
            }
        }
        // global
        void erase(RETURN (*func)(ARGS...)) {
            auto key = callback::key_callback_global<RETURN(ARGS...)>(func);

            for (std::size_t i = 0; i < callbacks.size(); ++i) {
                if(callbacks[i]->compare(key)) {
                    callbacks.erase(callbacks.begin() + i);
                    return;
                }
            }
        }
        // lambda
        template <std::uintptr_t KEY>
        void erase() {
            auto key = callback::key_callback_lambda(KEY);
            for (std::size_t i = 0; i < callbacks.size(); ++i) {
                if(callbacks[i]->compare(key)) {
                    callbacks.erase(callbacks.begin() + i);
                    return;
                }
            }
        }

        template <typename T> constexpr auto pick(RETURN (T::*func)(ARGS...)) -> decltype(func) { return func; }
    };

    template<typename... ARGS> 
    class action<void(ARGS...)> {
    public:
        using object_type = std::unique_ptr<callback::object_callback<void(ARGS...)>>;

        std::vector<object_type> callbacks;

        operator bool() { return !callbacks.empty(); }

        void invoke(ARGS... args) {
            if(callbacks.empty()) return;
            for (std::size_t i = 0; i < callbacks.size(); ++i) {
                callbacks[i]->invoke(args...);
            }
        }

        // member
        template<auto FUNC, typename T>
        void push_back(T* object) {
            using cb_t = callback::callback<FUNC, void(ARGS...)>;
            callbacks.push_back(std::make_unique<cb_t>(object));
        }
        // global
        template<auto FUNC>
        void push_back() {
            using cb_t = callback::callback<FUNC, void(ARGS...)>;
            callbacks.push_back(std::make_unique<cb_t>());
        }
        // lambda
        template<std::uintptr_t key, typename FUNC>
        void push_back(FUNC f) {
            using cb_t = callback::lambda_callback<FUNC, void(ARGS...), key>;
            callbacks.push_back(std::make_unique<cb_t>(std::move(f)));
        }

        // member
        template <typename T>
        void erase(void (T::*func)(ARGS...), const T* object) {
            auto key = callback::d_key_callback_member<decltype(func)>(
                reinterpret_cast<std::uintptr_t>(object),
                func
            );
            for (std::size_t i = 0; i < callbacks.size(); ++i) {
                if(callbacks[i]->compare(&key)) {
                    callbacks.erase(callbacks.begin() + i);
                    return;
                }
            }
        }
        // global
        void erase(void (*func)(ARGS...)) {
            auto key = callback::key_callback_global<void(ARGS...)>(func);

            for (std::size_t i = 0; i < callbacks.size(); ++i) {
                if(callbacks[i]->compare(key)) {
                    callbacks.erase(callbacks.begin() + i);
                    return;
                }
            }
        }
        // lambda
        template <std::uintptr_t KEY>
        void erase() {
            auto key = callback::key_callback_lambda(KEY);
            for (std::size_t i = 0; i < callbacks.size(); ++i) {
                if(callbacks[i]->compare(key)) {
                    callbacks.erase(callbacks.begin() + i);
                    return;
                }
            }
        }

        template <typename T> constexpr auto pick(void (T::*func)(ARGS...)) -> decltype(func) { return func; }
    };
}

#endif
