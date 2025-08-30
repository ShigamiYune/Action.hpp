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
#include <functional>
#include <type_traits>

namespace action {
#if __cplusplus >= 201703L
    namespace callback {
        template<typename T>
        struct get_type {
            using remove_all = 
                std::remove_cv_t<
                std::remove_reference_t<
                std::remove_pointer_t<
                std::remove_all_extents_t<T>>>>;
        };

        template <typename T>
        using remove_all_t = typename get_type<T>::remove_all;

        namespace key {
            enum type_key : std::uint8_t{
                member = 167, 
                global = 241,
                lambda = 110
            };

            class key_member_base {
            public:
                std::uintptr_t ptr;
                std::type_index func_type;

                key_member_base(std::uintptr_t obj_ptr, std::type_index _func_type)
                    : ptr(obj_ptr), func_type(_func_type) {}
            };

            template<typename FUNC_T>
            class key_member : public key_member_base {
            public:
                FUNC_T func_ptr;

                key_member(std::uintptr_t obj_ptr, FUNC_T fptr)
                    : key_member_base(obj_ptr, std::type_index(typeid(FUNC_T)))
                    , func_ptr(fptr)
                {}
            };

            template<typename SIGNATURE> struct key_global;
            template<typename RETURN, typename... ARGS>
            class key_global<RETURN(ARGS...)> {
            public:
                RETURN(*func_ptr)(ARGS...) = nullptr;
                RETURN(*func_ptr_noexcept)(ARGS...) noexcept = nullptr;
                key_global(RETURN(*func)(ARGS...) = nullptr, RETURN(*func_noexcept)(ARGS...) noexcept = nullptr) : 
                    func_ptr(func), func_ptr_noexcept(func_noexcept) {}
            };

            class key_lambda_base { 
            public:
                std::type_index type_key;
                explicit key_lambda_base(std::type_index _type_key) : type_key(_type_key) {}
                
                bool compare(const key_lambda_base& other) const {
                    return type_key == other.type_key;
                }
            };

            template<auto _key>
            class key_lambda : public key_lambda_base { 
            public:
                using key_type = std::decay_t<decltype(_key)>;
                key_type key;

                key_lambda() 
                    : key(_key)
                    , key_lambda_base(std::type_index(typeid(_key))) 
                {}
            };
        }

        namespace base {
            template<typename SIGNATURE> class object_callback;
            template<typename RETURN, typename... ARGS> 
            class object_callback<RETURN(ARGS...)> {
            public:
                virtual ~object_callback() = default;
                virtual RETURN invoke(ARGS...) = 0;
                virtual bool compare(std::uint8_t, const void*) = 0;
                RETURN operator()(ARGS... args) { return invoke(args...); }
            };
        }

        // ----- CALLBACK MEMBER --------------------------------------------------------
        template<typename CLASS, typename SIGNATURE, auto FUNC> class callback_member;

        template<typename CLASS, typename RETURN, typename... ARGS, auto FUNC>
        class callback_member<CLASS, RETURN(ARGS...), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            RETURN invoke(ARGS... args) override {
                if constexpr (std::is_pointer_v<std::remove_reference_t<CLASS>>) {
                    return (_object->*FUNC)(args...);
                } else {
                    return (_object.*FUNC)(args...);
                }
            }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        // ------------------------------------------------------------------------------

        // ----- CALLBACK GLOBAL --------------------------------------------------------
        template<typename SIGNATURE, auto FUNC> class callback_global;

        template<typename RETURN, typename... ARGS, auto FUNC>
        class callback_global<RETURN(ARGS...), FUNC> : public base::object_callback<RETURN(ARGS...)>{
        public: 
            RETURN invoke(ARGS... args) override { return (*FUNC)(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::global) return false;
                return reinterpret_cast<const key::key_global<RETURN(ARGS...)>*>(key_ptr)->func_ptr == FUNC;
            }
        };
        // ------------------------------------------------------------------------------

        // ----- CALLBACK LAMBDA --------------------------------------------------------
        template<typename LAMBDA_T, typename SIGNATURE, auto key> class callback_lambda;

        template<typename LAMBDA_T, auto key , typename RETURN, typename... ARGS>
        class callback_lambda<LAMBDA_T, RETURN(ARGS...), key> : public base::object_callback<RETURN(ARGS...)>{
            LAMBDA_T _lambda;
        public: 
            callback_lambda(const LAMBDA_T& lambda) : _lambda(lambda) {}
            callback_lambda(LAMBDA_T&& lambda) : _lambda(std::move(lambda)) {}

            RETURN invoke(ARGS... args) override { return _lambda(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::lambda) return false;
                const key::key_lambda_base* base = reinterpret_cast<const key::key_lambda_base*>(key_ptr);
                return 
                    (base->type_key == typeid(key)) 
                    && 
                    (reinterpret_cast<const key::key_lambda<key>*>(base)->key == key);
            }
        };
        // ------------------------------------------------------------------------------

        // ----- CALLBACK WRAP ---------------------------------------------------------
        template<typename CLASS, typename SIGNATURE, auto FUNC, typename WRAP> class callback_wrap;

        template<typename CLASS, auto FUNC, typename RETURN, typename... ARGS>
        class callback_wrap<CLASS, RETURN(ARGS...), FUNC, std::shared_ptr<CLASS>>
            : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };

        template<typename CLASS, auto FUNC, typename RETURN, typename... ARGS>
        class callback_wrap<CLASS, RETURN(ARGS...), FUNC, std::weak_ptr<CLASS>>
            : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        // ------------------------------------------------------------------------------
    }

    template<typename SIGNATURE> class object_callback;
    template<typename RETURN, typename... ARGS> 
    class object_callback<RETURN(ARGS...)> {
        using object_type = std::unique_ptr<callback::base::object_callback<RETURN(ARGS...)>>;
        template<typename CLASS, auto FUNC, typename WRAP>
        static object_type make_unique_member(WRAP&& object) {
            if constexpr (std::is_same_v<callback::remove_all_t<WRAP>, CLASS>) {
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), FUNC>;

                return std::make_unique<cb_t>(std::forward<WRAP>(object));
            }
            else if constexpr (std::is_same_v<std::weak_ptr<CLASS>, callback::remove_all_t<WRAP>> 
                || std::is_same_v<std::shared_ptr<CLASS>, callback::remove_all_t<WRAP>>) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), FUNC, WRAP>;
                return std::make_unique<cb_t>(std::forward<WRAP>(object));
            }
            else {
                static_assert(false, "object error: you can be change to use lambda");
            }
            return nullptr;
        };
    public:
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...), typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) noexcept, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const noexcept, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile noexcept, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile noexcept, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) & noexcept, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &&, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) && noexcept, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const && noexcept, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile & noexcept, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile && noexcept, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile & noexcept, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile && noexcept, typename WRAP>
        static object_type make_unique(WRAP&& object) { 
            return std::move(make_unique_member<CLASS, FUNC>(std::forward<WRAP>(object)));
        }

        template<RETURN(*FUNC)(ARGS...)>
        static object_type make_unique() {
            using cb_t = callback::callback_global<RETURN(ARGS...), FUNC>;
            return std::move(std::make_unique<cb_t>());
        }

        template<RETURN(*FUNC)(ARGS...) noexcept>
        static object_type make_unique() {
            using cb_t = callback::callback_global<RETURN(ARGS...), FUNC>;
            return std::move(std::make_unique<cb_t>());
        }

        template<auto key, typename FUNC_T>
        static object_type make_unique(FUNC_T&& func) {
            using cb_t = callback::callback_lambda<std::decay_t<FUNC_T>, RETURN(ARGS...), key>;
            return std::move(std::make_unique<cb_t>(std::forward<FUNC_T>(func)));
        }
        template<typename KEY_T, KEY_T key, typename FUNC_T>
        static object_type make_unique(FUNC_T&& func) {
            using cb_t = callback::callback_lambda<std::decay_t<FUNC_T>, RETURN(ARGS...), key>;
            return std::move(std::make_unique<cb_t>(std::forward<FUNC_T>(func)));
        }
    };

    // ----- action ---------------------------------------------------------------------
    template<typename SIGNATURE> class action;

    template<typename RETURN, typename... ARGS> 
    class action<RETURN(ARGS...)> {
        // member and wrap
        template<typename CLASS, auto FUNC, typename WRAP>
        void push_back_member(WRAP&& object) {
            if constexpr (std::is_same_v<callback::remove_all_t<WRAP>, CLASS>) {
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), FUNC>;
                callbacks.push_back(std::make_unique<cb_t>(object));
            }
            else if constexpr (std::is_same_v<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>> 
                || std::is_same_v<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), FUNC, callback::remove_all_t<WRAP>>;
                callbacks.push_back(std::make_unique<cb_t>(std::forward<WRAP>(object)));
            }
            else {
                static_assert(false, "object error: you can be change to use lambda");
            }
        }

        // member and wrap
        template<typename CLASS, auto FUNC, typename WRAP>
        void erase_member(WRAP&& object) {
            using key_type = callback::key::key_member<decltype(FUNC)>;
            key_type member_key(0, FUNC);

            if constexpr (std::is_same_v<callback::remove_all_t<WRAP>, CLASS>) {
                member_key.ptr = reinterpret_cast<std::uintptr_t>(object);
            }
            else if constexpr (std::is_same_v<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>) {
                member_key.ptr = reinterpret_cast<std::uintptr_t>(object.get());
            }
            else if constexpr (std::is_same_v<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>) {
                auto sp = object.lock();
                if (sp) member_key.ptr = reinterpret_cast<std::uintptr_t>(sp.get());
            }
            else {
                static_assert(false, "erase_member: unsupported object type");
            }

            // Duyệt và xóa callback
            for (auto it = callbacks.begin(); it != callbacks.end(); ++it) {
                if ((*it)->compare(callback::key::member, &member_key)) {
                    callbacks.erase(it);
                    return;
                }
            }
        }
    public:
        using object_type = std::unique_ptr<callback::base::object_callback<RETURN(ARGS...)>>;

        std::vector<object_type> callbacks;

        operator bool() { return !callbacks.empty(); }

        RETURN invoke(ARGS... args) {
            if(callbacks.empty()) {
                if constexpr (std::is_same_v<void, RETURN>) {
                    return;
                }
                else {
                    return {};
                }
            }
            for (std::size_t i = 0; i < callbacks.size() - 1; ++i) {
                callbacks[i]->invoke(args...);
            }
            return callbacks[callbacks.size() - 1]->invoke(args...);
        }
        RETURN operator()(ARGS... args) { return invoke(args...); }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...), typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) noexcept, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const noexcept, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile noexcept, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile noexcept, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) & noexcept, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &&, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) && noexcept, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const && noexcept, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile & noexcept, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile && noexcept, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile & noexcept, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile && noexcept, typename WRAP>
        void push_back(WRAP&& object) { 
            push_back_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<RETURN(*FUNC)(ARGS...)>
        void push_back() {
            using cb_t = callback::callback_global<RETURN(ARGS...), FUNC>;
            callbacks.push_back(std::make_unique<cb_t>());
        }

        template<RETURN(*FUNC)(ARGS...) noexcept>
        void push_back() {
            using cb_t = callback::callback_global<RETURN(ARGS...), FUNC>;
            callbacks.push_back(std::make_unique<cb_t>());
        }

        template<auto key, typename FUNC_T>
        void push_back(FUNC_T&& func) {
            using cb_t = callback::callback_lambda<std::decay_t<FUNC_T>, RETURN(ARGS...), key>;
            callbacks.push_back(std::make_unique<cb_t>(std::forward<FUNC_T>(func)));
        }
        template <typename KEY_T, KEY_T key, typename FUNC_T>
        void push_back(FUNC_T&& func) { 
            using cb_t = callback::callback_lambda<std::decay_t<FUNC_T>, RETURN(ARGS...), key>;
            callbacks.push_back(std::make_unique<cb_t>(std::forward<FUNC_T>(func)));
        }
        
        // member
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...), typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) noexcept, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const noexcept, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile noexcept, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile noexcept, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) & noexcept, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &&, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) && noexcept, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const && noexcept, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile & noexcept, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile && noexcept, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile & noexcept, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile && noexcept, typename WRAP>
        void erase(WRAP&& object) { 
            erase_member<CLASS, FUNC>(std::forward<WRAP>(object));
        }
        // global
        template <RETURN(*FUNC)(ARGS...)>
        void erase() {
            auto key = callback::key::key_global<RETURN(ARGS...)>(FUNC, nullptr);

            for (std::size_t i = 0; i < callbacks.size(); ++i) {
                if(callbacks[i]->compare(callback::key::global, &key)) {
                    callbacks.erase(callbacks.begin() + i);
                    return;
                }
            }
        }
        template <RETURN(*FUNC)(ARGS...) noexcept>
        void erase() {
            auto key = callback::key::key_global<RETURN(ARGS...)>(nullptr, FUNC);

            for (std::size_t i = 0; i < callbacks.size(); ++i) {
                if(callbacks[i]->compare(callback::key::global, &key)) {
                    callbacks.erase(callbacks.begin() + i);
                    return;
                }
            }
        }
        // lambda
        template <auto KEY, typename = typename std::enable_if<std::is_member_function_pointer<decltype(KEY)>::value>::type>
        void erase() {
            auto key = callback::key::key_lambda<KEY>();
            for (std::size_t i = 0; i < callbacks.size(); ++i) {
                if(callbacks[i]->compare(callback::key::lambda, &key)) {
                    callbacks.erase(callbacks.begin() + i);
                    return;
                }
            }
        }
        template <typename KEY_T, KEY_T _key>
        void erase() { 
            auto key = callback::key::key_lambda<_key>();
            for (std::size_t i = 0; i < callbacks.size(); ++i) {
                if(callbacks[i]->compare(callback::key::lambda, &key)) {
                    callbacks.erase(callbacks.begin() + i);
                    return;
                }
            }
        }
    };
#elif __cplusplus >= 201103L
    namespace callback {
        template<typename T> struct get_type {
            using remove_all = typename
                std::remove_cv< typename
                std::remove_reference< typename
                std::remove_pointer< typename
                std::remove_all_extents<T>::type>::type>::type>::type;
        };
        template <typename T> using remove_all_t = typename get_type<T>::remove_all;
        namespace key {
            enum type_key : std::uint8_t{
                member = 167, 
                global = 241,
                lambda = 110
            };

            class key_member_base {
            public:
                std::uintptr_t ptr;
                std::type_index func_type;

                key_member_base(std::uintptr_t obj_ptr, std::type_index _func_type)
                    : ptr(obj_ptr), func_type(_func_type) {}
            };

            template<typename FUNC_T>
            class key_member : public key_member_base {
            public:
                FUNC_T func_ptr;

                key_member(std::uintptr_t obj_ptr, FUNC_T fptr)
                    : key_member_base(obj_ptr, std::type_index(typeid(FUNC_T)))
                    , func_ptr(fptr)
                {}
            };

            template<typename SIGNATURE> struct key_global;
            template<typename RETURN, typename... ARGS>
            class key_global<RETURN(ARGS...)> {
            public:
                RETURN(*func_ptr)(ARGS...) = nullptr;

                key_global(RETURN(*func)(ARGS...) = nullptr) : func_ptr(func){}
            };

            class key_lambda_base { 
            public:
                std::type_index type_key;
                explicit key_lambda_base(std::type_index _type_key) : type_key(_type_key) {}
                
                bool compare(const key_lambda_base& other) const {
                    return type_key == other.type_key;
                }
            };

            template<typename KEY_T, KEY_T _key>
            class key_lambda : public key_lambda_base { 
            public:
                KEY_T key;

                key_lambda() 
                    : key(_key)
                    , key_lambda_base(std::type_index(typeid(_key))) 
                {}
            };
        }
        namespace base {
            template<typename SIGNATURE> class object_callback;
            template<typename RETURN, typename... ARGS> 
            class object_callback<RETURN(ARGS...)> {
            public:
                virtual ~object_callback() = default;
                virtual RETURN invoke(ARGS...) = 0;
                virtual bool compare(std::uint8_t, const void*) = 0;
                RETURN operator()(ARGS... args) { return invoke(args...); }
            };
        }

        // ----- CALLBACK MEMBER --------------------------------------------------------
        template<typename CLASS, typename SIGNATURE, typename FUNC_T, FUNC_T FUNC> class callback_member;
        
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(remove_all_t<CLASS>::*FUNC)(ARGS...)>
        class callback_member<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            template<typename C = CLASS>
            typename std::enable_if<std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object->*FUNC)(args...); }
            template<typename C = CLASS>
            typename std::enable_if<!std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object.*FUNC)(args...); }

            RETURN invoke(ARGS... args) override { return invoke_impl(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(remove_all_t<CLASS>::*FUNC)(ARGS...) const>
        class callback_member<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            template<typename C = CLASS>
            typename std::enable_if<std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object->*FUNC)(args...); }
            template<typename C = CLASS>
            typename std::enable_if<!std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object.*FUNC)(args...); }

            RETURN invoke(ARGS... args) override { return invoke_impl(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(remove_all_t<CLASS>::*FUNC)(ARGS...) volatile>
        class callback_member<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            template<typename C = CLASS>
            typename std::enable_if<std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object->*FUNC)(args...); }
            template<typename C = CLASS>
            typename std::enable_if<!std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object.*FUNC)(args...); }

            RETURN invoke(ARGS... args) override { return invoke_impl(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(remove_all_t<CLASS>::*FUNC)(ARGS...) const volatile>
        class callback_member<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            template<typename C = CLASS>
            typename std::enable_if<std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object->*FUNC)(args...); }
            template<typename C = CLASS>
            typename std::enable_if<!std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object.*FUNC)(args...); }

            RETURN invoke(ARGS... args) override { return invoke_impl(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(remove_all_t<CLASS>::*FUNC)(ARGS...) &>
        class callback_member<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            template<typename C = CLASS>
            typename std::enable_if<std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object->*FUNC)(args...); }
            template<typename C = CLASS>
            typename std::enable_if<!std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object.*FUNC)(args...); }

            RETURN invoke(ARGS... args) override { return invoke_impl(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(remove_all_t<CLASS>::*FUNC)(ARGS...) const &>
        class callback_member<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            template<typename C = CLASS>
            typename std::enable_if<std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object->*FUNC)(args...); }
            template<typename C = CLASS>
            typename std::enable_if<!std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object.*FUNC)(args...); }

            RETURN invoke(ARGS... args) override { return invoke_impl(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(remove_all_t<CLASS>::*FUNC)(ARGS...) volatile &>
        class callback_member<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            template<typename C = CLASS>
            typename std::enable_if<std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object->*FUNC)(args...); }
            template<typename C = CLASS>
            typename std::enable_if<!std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object.*FUNC)(args...); }

            RETURN invoke(ARGS... args) override { return invoke_impl(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(remove_all_t<CLASS>::*FUNC)(ARGS...) const volatile &>
        class callback_member<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            template<typename C = CLASS>
            typename std::enable_if<std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object->*FUNC)(args...); }
            template<typename C = CLASS>
            typename std::enable_if<!std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object.*FUNC)(args...); }

            RETURN invoke(ARGS... args) override { return invoke_impl(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(remove_all_t<CLASS>::*FUNC)(ARGS...) &&>
        class callback_member<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            template<typename C = CLASS>
            typename std::enable_if<std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object->*FUNC)(args...); }
            template<typename C = CLASS>
            typename std::enable_if<!std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object.*FUNC)(args...); }

            RETURN invoke(ARGS... args) override { return invoke_impl(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };        
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(remove_all_t<CLASS>::*FUNC)(ARGS...) const &&>
        class callback_member<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            template<typename C = CLASS>
            typename std::enable_if<std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object->*FUNC)(args...); }
            template<typename C = CLASS>
            typename std::enable_if<!std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object.*FUNC)(args...); }

            RETURN invoke(ARGS... args) override { return invoke_impl(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(remove_all_t<CLASS>::*FUNC)(ARGS...) volatile &&>
        class callback_member<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            template<typename C = CLASS>
            typename std::enable_if<std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object->*FUNC)(args...); }
            template<typename C = CLASS>
            typename std::enable_if<!std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object.*FUNC)(args...); }

            RETURN invoke(ARGS... args) override { return invoke_impl(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(remove_all_t<CLASS>::*FUNC)(ARGS...) const volatile &&>
        class callback_member<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
            CLASS _object;
        public: 
            callback_member(const CLASS& object) : _object(object) {}
            callback_member(CLASS&& object) : _object(std::move(object)) {}

            template<typename C = CLASS>
            typename std::enable_if<std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object->*FUNC)(args...); }
            template<typename C = CLASS>
            typename std::enable_if<!std::is_pointer<typename std::remove_reference<C>::type>::value, RETURN>::type
            invoke_impl(ARGS... args) { return (_object.*FUNC)(args...); }

            RETURN invoke(ARGS... args) override { return invoke_impl(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_object) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };

        // ----- CALLBACK GLOBAL --------------------------------------------------------
        template<typename SIGNATURE, typename FUNC_T, FUNC_T FUNC> class callback_global;
        template<typename RETURN, typename... ARGS, RETURN(*FUNC)(ARGS...)>
        class callback_global<RETURN(ARGS...), decltype(FUNC), FUNC> : public base::object_callback<RETURN(ARGS...)>{
        public: 
            RETURN invoke(ARGS... args) override { return (*FUNC)(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::global) return false;
                return reinterpret_cast<const key::key_global<RETURN(ARGS...)>*>(key_ptr)->func_ptr == FUNC;
            }
        };
        
        // ------------------------------------------------------------------------------

        // ----- CALLBACK LAMBDA --------------------------------------------------------
        template<typename LAMBDA_T, typename SIGNATURE, typename KEY_T, KEY_T key> class callback_lambda;
        template<typename LAMBDA_T, typename KEY_T, KEY_T key, typename RETURN, typename... ARGS>
        class callback_lambda<LAMBDA_T, RETURN(ARGS...), KEY_T, key> : public base::object_callback<RETURN(ARGS...)>{
            LAMBDA_T _lambda;
        public: 
            callback_lambda(const LAMBDA_T& lambda) : _lambda(lambda) {}
            callback_lambda(LAMBDA_T&& lambda) : _lambda(std::move(lambda)) {}

            RETURN invoke(ARGS... args) override { return _lambda(args...); }

            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::lambda) return false;
                const key::key_lambda_base* base = reinterpret_cast<const key::key_lambda_base*>(key_ptr);
                return 
                    (base->type_key == typeid(key)) 
                    && 
                    (reinterpret_cast<const key::key_lambda<KEY_T, key>*>(base)->key == key);
            }
        };
        // ------------------------------------------------------------------------------

        // ----- CALLBACK WRAP ---------------------------------------------------------
        template<typename CLASS, typename SIGNATURE, typename FUNC_T, FUNC_T FUNC, typename WRAP> class callback_wrap;

        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...)>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) const>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) volatile>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) const volatile>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) &>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) const &>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) volatile &>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) &&>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) const &&>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::shared_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::shared_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::shared_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override { 
                return (_wrap.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (reinterpret_cast<std::uintptr_t>(_wrap.get()) == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) >
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) const >
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) volatile >
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) const volatile>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) & >
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) const & >
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) volatile & >
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) const volatile & >
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) && >
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) const &&>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) volatile && >
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        template<typename CLASS, typename RETURN, typename... ARGS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&>
        class callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>> : public base::object_callback<RETURN(ARGS...)>{
            std::weak_ptr<CLASS> _wrap;
        public: 
            callback_wrap(const std::weak_ptr<CLASS>& wrap) : _wrap(wrap) {}
            callback_wrap(std::weak_ptr<CLASS>&& wrap) : _wrap(std::move(wrap)) {}

            RETURN invoke(ARGS... args) override {
                auto sp = _wrap.lock();
                if (!sp) throw std::bad_function_call();
                return (sp.get()->*FUNC)(args...);
            }
            
            bool compare(std::uint8_t type, const void* key_ptr) override {
                if(type != key::member) return false;
                std::uintptr_t ptr = 0;
                
                auto sp = _wrap.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                const key::key_member_base* key = reinterpret_cast<const key::key_member_base*>(key_ptr);
                return 
                    (ptr == key->ptr) 
                    && 
                    (std::type_index(typeid(decltype(FUNC))) == key->func_type)
                    &&
                    (static_cast<const key::key_member<decltype(FUNC)>*>(key)->func_ptr == FUNC)
                    ;
            }
        };
        
        // ------------------------------------------------------------------------------
    }

    template<typename SIGNATURE> class object_callback;
    template<typename RETURN, typename... ARGS> 
    class object_callback<RETURN(ARGS...)> {
    public:
        using object_type = std::unique_ptr<callback::base::object_callback<RETURN(ARGS...)>>;

        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...), typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...), typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...), typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
        static object_type make_unique(WRAP&& object) { 
            using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
        }
        
        template<RETURN(*FUNC)(ARGS...)>
        static object_type make_unique() {
            using cb_t = callback::callback_global<RETURN(ARGS...), decltype(FUNC), FUNC>;
            return std::move(std::unique_ptr<cb_t>(new cb_t()));
        }

        template<typename KEY_T, KEY_T key, typename FUNC_T>
        static object_type make_unique(FUNC_T&& func) {
            using cb_t = callback::callback_lambda<typename std::decay<FUNC_T>::type, RETURN(ARGS...),KEY_T, key>;
            return std::move(std::unique_ptr<cb_t>(new cb_t(std::forward<FUNC_T>(func))));
        }
    };

    // ----- action ---------------------------------------------------------------------
        template<typename SIGNATURE> class action;
        template<typename RETURN, typename... ARGS> 
        class action<RETURN(ARGS...)> {
            void _erase(callback::key::type_key type_key, const void* key_ptr){
                for (std::size_t i = 0; i < callbacks.size(); ++i) {
                    if(callbacks[i]->compare(callback::key::global, key_ptr)) {
                        callbacks.erase(callbacks.begin() + i);
                        return;
                    }
                }
            }
        public:
            using object_type = std::unique_ptr<callback::base::object_callback<RETURN(ARGS...)>>;

            std::vector<object_type> callbacks;

            operator bool() { return !callbacks.empty(); }

            template<typename R = RETURN>
            typename std::enable_if<std::is_same<R, void>::value, void>::type
            invoke(ARGS... args) {
                if (callbacks.empty()) return;
                for (std::size_t i = 0; i < callbacks.size() - 1; ++i)
                    callbacks[i]->invoke(args...);
                callbacks.back()->invoke(args...);
            }

            template<typename R = RETURN>
            typename std::enable_if<!std::is_same<R, void>::value, R>::type
            invoke(ARGS... args) {
                if (callbacks.empty()) return R();
                for (std::size_t i = 0; i < callbacks.size() - 1; ++i)
                    callbacks[i]->invoke(args...);
                return callbacks.back()->invoke(args...);
            }
            RETURN operator()(ARGS... args) { return invoke(args...); }

            // MEMBER ------------------------------------------------------------------------------------
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...), typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_member<WRAP, RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            
            // WRAP ------------------------------------------------------------------------------------
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...), typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) { 
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::shared_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...), typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void push_back(WRAP&& object) {
                using cb_t = callback::callback_wrap<CLASS, RETURN(ARGS...), decltype(FUNC), FUNC, std::weak_ptr<CLASS>>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<WRAP>(object))));
            }

            // GLOBAL ------------------------------------------------------------------------------------
            template<RETURN(*FUNC)(ARGS...)>
            void push_back() {
                using cb_t = callback::callback_global<RETURN(ARGS...), decltype(FUNC), FUNC>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t()));
            }

            template<typename KEY_T, KEY_T key, typename FUNC_T>
            void push_back(FUNC_T&& func) {
                using cb_t = callback::callback_lambda<typename std::decay<FUNC_T>::type, RETURN(ARGS...), KEY_T, key>;
                callbacks.push_back(std::unique_ptr<cb_t>(new cb_t(std::forward<FUNC_T>(func))));
            }
            
            // member
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...), typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, CLASS>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object), FUNC);
                _erase(callback::key::member, &key);
            }
            
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...), typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object.get()), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object.get()), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object.get()), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object.get()), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object.get()), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object.get()), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object.get()), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object.get()), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object.get()), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object.get()), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object.get()), FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::shared_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) { 
                auto key = callback::key::key_member<decltype(FUNC)>(reinterpret_cast<std::uintptr_t>(object.get()), FUNC);
                _erase(callback::key::member, &key);
            }
            
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...), typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) {
                std::uintptr_t ptr = 0;
                auto sp = object.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                auto key = callback::key::key_member<decltype(FUNC)>(ptr, FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) {
                std::uintptr_t ptr = 0;
                auto sp = object.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                auto key = callback::key::key_member<decltype(FUNC)>(ptr, FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) {
                std::uintptr_t ptr = 0;
                auto sp = object.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                auto key = callback::key::key_member<decltype(FUNC)>(ptr, FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) {
                std::uintptr_t ptr = 0;
                auto sp = object.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                auto key = callback::key::key_member<decltype(FUNC)>(ptr, FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) {
                std::uintptr_t ptr = 0;
                auto sp = object.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                auto key = callback::key::key_member<decltype(FUNC)>(ptr, FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) {
                std::uintptr_t ptr = 0;
                auto sp = object.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                auto key = callback::key::key_member<decltype(FUNC)>(ptr, FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) {
                std::uintptr_t ptr = 0;
                auto sp = object.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                auto key = callback::key::key_member<decltype(FUNC)>(ptr, FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) {
                std::uintptr_t ptr = 0;
                auto sp = object.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                auto key = callback::key::key_member<decltype(FUNC)>(ptr, FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) {
                std::uintptr_t ptr = 0;
                auto sp = object.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                auto key = callback::key::key_member<decltype(FUNC)>(ptr, FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) {
                std::uintptr_t ptr = 0;
                auto sp = object.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                auto key = callback::key::key_member<decltype(FUNC)>(ptr, FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) {
                std::uintptr_t ptr = 0;
                auto sp = object.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                auto key = callback::key::key_member<decltype(FUNC)>(ptr, FUNC);
                _erase(callback::key::member, &key);
            }
            template<typename CLASS, RETURN(CLASS::*FUNC)(ARGS...) const volatile &&, typename WRAP,
                typename std::enable_if<std::is_same<callback::remove_all_t<WRAP>, std::weak_ptr<CLASS>>::value, int>::type = 0>
            void erase(WRAP&& object) {
                std::uintptr_t ptr = 0;
                auto sp = object.lock();
                if (sp) ptr = reinterpret_cast<std::uintptr_t>(sp.get());

                auto key = callback::key::key_member<decltype(FUNC)>(ptr, FUNC);
                _erase(callback::key::member, &key);
            }
            
            // global
            template <RETURN (*FUNC)(ARGS...)>
            void erase() {
                auto key = callback::key::key_global<RETURN(ARGS...)>(FUNC);
                _erase(callback::key::global, &key);
            }
            // lambda
            template <typename KEY_T, KEY_T _key>
            void erase() {
                auto key = callback::key::key_lambda<KEY_T, _key>();
                _erase(callback::key::lambda, &key);
            }
        };
#endif
}
#endif
