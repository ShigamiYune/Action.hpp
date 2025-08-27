# Action

## Memory Usage on 64-bit Systems

| Callback Type         | Estimated Memory Usage             |
|-----------------------|------------------------------------|
| Member Function        | 24 bytes (16 heap + 8 ptr in Action)   |
| Static/Global Function | 16 bytes (8 heap + 8 ptr in Action)    |
| Lambda (with capture)  | 16 (8 heap + 8 ptr in Action) + size of lambda (aligned)      |

---

## Example Usage

```cpp
#include "Action.hpp"
#include <iostream>

struct MyClass {
    void member(int x) { std::cout << "Member: " << x << "\n"; }
    void member_double(double x) { std::cout << "Member double: " << x << "\n"; }
    void member_const(int x) const { std::cout << "Const Member: " << x << "\n"; }
    static void _static(int x) { std::cout << "Static: " << x << "\n"; }
};

void global(int x) { std::cout << "Global: " << x << "\n"; }
static void global_static(int x) { std::cout << "Global Static: " << x << "\n"; }

int global_return(int x) { std::cout << "Global return: " << x << "\n"; return x*2; }

int main() {
    MyClass obj;
    action::action<void(int)> onEvent;
    action::action<int(int)> onEventReturn;

    // ==== Member function ====
    onEvent += action::make_callback<&MyClass::member>(&obj);

    // ==== Const member function ====
    const MyClass const_obj;
    onEvent += action::make_callback<&MyClass::member_const>(&const_obj);

    // ==== Member function overload ==== that is error
    // onEvent += action::make_callback<&MyClass::member_double>(&obj); 

    // ==== Static / global ====
    onEvent += action::make_callback<&MyClass::_static>();
    onEvent += action::make_callback<&global>();
    onEvent += action::make_callback<&global_static>();

    // ==== Lambda ====
    int capture_val = 5;
    auto lambda = [capture_val](int x){ std::cout << "Lambda capture: " << x + capture_val << "\n"; };
    onEvent += action::make_callback<42>(lambda);

    auto lambda_mut = [y=10](int x) mutable { std::cout << "Lambda mutable: " << x + y << "\n"; };
    onEvent += action::make_callback<43>(lambda_mut);

    // ==== Function with return ====
    onEventReturn += action::make_callback<&global_return>();
    
    // ==== Invoke ====
    std::cout << "Invoke all callbacks with 10:\n";
    onEvent.invoke(10);

    std::cout << "\nInvoke return callback with 10:\n";
    int ret = onEventReturn.invoke(10);
    std::cout << "Return value: " << ret << "\n";

    // ==== Test remove callbacks ====
    onEvent -= action::get_key_callback<&MyClass::member>(&obj);
    onEvent -= action::get_key_callback<&MyClass::_static>();
    onEvent -= action::get_key_callback<42>();

    std::cout << "\nInvoke after removing some callbacks:\n";
    onEvent.invoke(20);

    // ==== Test multiple same key ====
    onEvent += action::make_callback<&global>();
    onEvent += action::make_callback<&global>();
    onEvent -= action::get_key_callback<&global>(); // should remove first occurrence

    std::cout << "\nInvoke after removing one of multiple globals:\n";
    onEvent.invoke(30);

    return 0;
}
```
