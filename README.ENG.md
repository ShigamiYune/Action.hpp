# Action.hpp

## Overview

`Action.hpp` is a **header-only, type-safe delegate/event system** for C++. It allows you to register callbacks (member functions, const members, global/static functions, and lambdas), invoke them, and remove them safely. It is lightweight and inspired by C# delegates.

---

## Required : C++ 11

---

## Memory Usage on 64-bit Systems

| Callback Type          | Estimated Memory Usage                                   |
| ---------------------- | -------------------------------------------------------- |
| Member Function        | 24 bytes (16 heap + 8 ptr in action::action)                     |
| Static/Global Function | 16 bytes (8 heap + 8 ptr in action::action)                      |
| Lambda (with or without capture)  | 16 (8 heap + 8 ptr in action::action) + size of lambda (aligned) |

---

## Public API

* `action::action<Signature>` — a container holding callbacks.
* `action::action<Signature>::push_back<Class, &Class::func>(object)` — add a callback from a member or const member function.
* `action::action<Signature>::push_back<&GlobalFunc>()` — add a callback from a global or static function.
* `action::action<Signature>::push_back<KEY>(lambda)` — add a callback (with or without capture); requires a unique key to remove it later.
* `action::action<Signature>::erase<Class, &Class::func>(object)` — remove a callback from a member or const member function.
* `action::action<Signature>::erase<&GlobalFunc>()` — remove a global or static function callback.
* `action::action<Signature>::erase<KEY>()` — remove a callback using its key.
* `action::object_callbaclk<Signature>::make_unique<...>()` — make a callback object
* `invoke(args...)` — execute all registered callbacks.

---

## Example Usage

```cpp
#include "Action.hpp"
#include <iostream>

struct MyClass {
    void member(int x) { std::cout << "Member: " << x << "\n"; }
    void member(unsigned long long _double) { std::cout << "Member: " << _double << "\n"; }

    void member_const(int x) const { std::cout << "Const member: " << x << "\n"; }
    static void _static(int x) { std::cout << "Static function: " << x << "\n"; }

    void shared_callback(int x) { std::cout << "shared_ptr: " << x << "\n"; }
    void weak_callback(int x) { std::cout << "weak_ptr: " << x << "\n"; }

    void object_callback(int x) { std::cout << "Callback from object: " << x << "\n"; }
};

// Global function
void global(int x) { std::cout << "Global function: " << x << "\n"; }

int main() {
    MyClass obj;
    const MyClass const_obj;
    action::action<void(int)> onEvent;         // event with no return value
    action::action<int(int)> onEventReturn;    // event returning int

    // Create callback directly from object
    auto my_callback = 
        action::object_callback<void(int)>::make_unique<MyClass, &MyClass::object_callback>(&obj);

    // Add callback from member function
    onEvent.push_back<MyClass, &MyClass::member>(&obj);

    // Add callback from const member function
    onEvent.push_back<MyClass, &MyClass::member_const>(&const_obj);

    // Add callback from static / global function
    onEvent.push_back<&MyClass::_static>();
    onEvent.push_back<&global>();

    // Add lambda with capture
    int val = 5;
    auto lambda = [val](int x){ std::cout << "Lambda capture: " << x + val << "\n"; };
    onEvent.push_back<int, 42>(lambda);

    // Add callback returning a value
    auto global_return = [](int x){ 
        std::cout << "Return: " << x << "\n"; 
        return x*2; 
    };
    onEventReturn.push_back<int, 24>(global_return);

    // Invoke all callbacks
    std::cout << "Invoking all callbacks:\n";
    onEvent.invoke(10);

    // Invoke callback returning a value
    std::cout << "\nInvoking callbacks with return value:\n";
    int ret = onEventReturn.invoke(10);
    std::cout << "Return value: " << ret << "\n";

    // Add callback using shared_ptr
    auto sp_obj = std::make_shared<MyClass>();
    onEvent.push_back<MyClass, &MyClass::shared_callback>(sp_obj);

    // Add callback using weak_ptr
    std::weak_ptr<MyClass> wp_obj = sp_obj;
    onEvent.push_back<MyClass, &MyClass::weak_callback>(wp_obj);

    // Erase callbacks by key
    onEvent.erase<MyClass, &MyClass::member>(&obj);
    onEvent.erase<int, 42>();

    std::cout << "\nInvoking after erasing some callbacks:\n";
    onEvent.invoke(20);

    // Invoke callback directly from object
    my_callback->invoke(30);
    
    return 0;
}
```

---

## Notes & Tips

1. **Lambda keys**

   * Every lambda (with capture and without capture) must have a unique key (`uintptr_t`) for removal.

2. **Thread-safety**

   * `action::action` is **not thread-safe**. Protect with a mutex if accessed from multiple threads.

3. **Return value behavior**

   * For non-void `action`, `invoke` returns the value of the **last callback**; all previous callbacks are invoked but their return values are discarded.

4. **Multiple arguments**

   * `action` supports any signature, e.g., `void(int, double)` or `int(std::string, float)`.
