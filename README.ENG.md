# Action.hpp

## Overview

`Action.hpp` is a **header-only, type-safe delegate/event system** for C++. It allows you to register callbacks (member functions, const members, global/static functions, and lambdas), invoke them, and remove them safely. It is lightweight and inspired by C# delegates.

---

## Memory Usage on 64-bit Systems

| Callback Type          | Estimated Memory Usage                                   |
| ---------------------- | -------------------------------------------------------- |
| Member Function        | 24 bytes (16 heap + 8 ptr in Action)                     |
| Static/Global Function | 16 bytes (8 heap + 8 ptr in Action)                      |
| Lambda (with capture)  | 16 (8 heap + 8 ptr in Action) + size of lambda (aligned) |

---

## Public API

* `action::action<Signature>` — container for callbacks.
* `action::make_callback<&Class::func>(object)` — create a member/const member callback.
* `action::make_callback<&GlobalFunc>()` — create a global/static function callback.
* `action::make_callback<KEY>(lambda)` — create a lambda callback with capture.
* `action::get_key_callback<&Class::func>(object)` — generate a key to remove callbacks.
* `operator+=` — add a callback.
* `operator-=` — remove a callback by key.
* `invoke(args...)` — call all registered callbacks.

---

## Example Usage

```cpp
#include "Action.hpp"
#include <iostream>

struct MyClass {
    void member(int x) { std::cout << "Member: " << x << "\n"; }
    void member_const(int x) const { std::cout << "Const Member: " << x << "\n"; }
    static void _static(int x) { std::cout << "Static: " << x << "\n"; }
};

void global(int x) { std::cout << "Global: " << x << "\n"; }

int main() {
    MyClass obj;
    const MyClass const_obj;
    action::action<void(int)> onEvent;
    action::action<int(int)> onEventReturn;

    // Add member callback
    onEvent += action::make_callback<&MyClass::member>(&obj);

    // Add const member callback
    onEvent += action::make_callback<&MyClass::member_const>(&const_obj);

    // Add static/global callbacks
    onEvent += action::make_callback<&MyClass::_static>();
    onEvent += action::make_callback<&global>();

    // Add lambda with capture
    int val = 5;
    auto lambda = [val](int x){ std::cout << "Lambda capture: " << x + val << "\n"; };
    onEvent += action::make_callback<42>(lambda);

    // Add callback with return
    auto global_return = [](int x){ std::cout << "Return: " << x << "\n"; return x*2; };
    onEventReturn += action::make_callback<global_return>();

    // Invoke all callbacks
    std::cout << "Invoke all callbacks:\n";
    onEvent.invoke(10);

    // Invoke return callbacks
    std::cout << "\nInvoke return callback:\n";
    int ret = onEventReturn.invoke(10);
    std::cout << "Return value: " << ret << "\n";

    // Remove callbacks using keys
    onEvent -= action::get_key_callback<&MyClass::member>(&obj);
    onEvent -= action::get_key_callback<42>();

    std::cout << "\nInvoke after removing some callbacks:\n";
    onEvent.invoke(20);

    return 0;
}
```

---

## Notes & Tips

1. **Lambda keys**

   * Every lambda with capture must have a unique key (`uintptr_t`) for removal.

2. **Overloaded member functions**

   * Template cannot automatically resolve overloads; use explicit cast if needed.

3. **Thread-safety**

   * `action::action` is **not thread-safe**. Protect with a mutex if accessed from multiple threads.

4. **Return value behavior**

   * For non-void `action`, `invoke` returns the value of the **last callback**; all previous callbacks are invoked but their return values are discarded.

5. **Multiple arguments**

   * `action` supports any signature, e.g., `void(int, double)` or `int(std::string, float)`.

---

## Callback Lifecycle (Flow)

```
+-------------------+
| Create Callback    |
| make_callback()    |
+-------------------+
          |
          v
+-------------------+
| Add to Action      |
| operator+=         |
+-------------------+
          |
          v
+-------------------+
| Invoke All         |
| invoke(args...)    |
+-------------------+
          |
          v
+-------------------+
| Remove Callback    |
| operator-= / key   |
+-------------------+
```

---

Mình có thể làm thêm **version GitHub-ready với folder `examples/` và `tests/`** nếu bạn muốn, để cộng đồng có thể clone và chạy ngay.

Bạn có muốn mình làm luôn không?
