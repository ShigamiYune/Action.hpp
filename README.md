# Action.hpp  
# Delegate/Event System in C++ — Usage Guide

The `Action` system below allows you to register multiple callbacks just like delegates in C#. Each `Action<Signature>` can hold multiple `ObjectEvent<Signature>` instances and invokes them sequentially when triggered.

---

## Main Components

- `Action<Return(Args...)>`: Manages the list of registered callbacks (delegate-like).

- `ObjectEvent<Return(Args...)>`: Abstract interface for encapsulated callbacks.

- `EVENT_*(...)` macros: Create callback instances from global, member, static, or lambda functions.

- `GET_KEY_EVENT_*(...)` macros: Generate a unique key (pointer + name) for identifying or removing specific callbacks.

---

## Features

- C#-like delegate/event API in C++
- Supports lambdas, member functions, static functions, and global functions
- Allows handler lookup and removal via `KeyEvent`
- Lightweight and efficient memory usage

---

## Memory Usage on 64-bit Systems

| Callback Type         | Estimated Memory Usage             |
|-----------------------|------------------------------------|
| Member Function        | 24 bytes (16 heap + 8 in Action)   |
| Static/Global Function | 16 bytes (8 heap + 8 in Action)    |
| Lambda (with capture)  | 24 + size of lambda (aligned)      |

---

## Example Usage

### 1. Define class using `Action`:

```cpp
#include "Action.hpp"

// Global function
int global_add(int x, int y) {
    std::cout << "global_add: " << x << " + " << y << " = " << x + y << std::endl;
    return x + y;
}

// Static function
struct Math {
    static int multiply(int x, int y) {
        std::cout << "Math::multiply: " << x << " * " << y << " = " << x * y << std::endl;
        return x * y;
    }
};

// A sample class with a member function
class MyClass {
public:
    int offset = 10;
    int accumulate(int x, int y) {
        int result = x + y + offset;
        std::cout << "MyClass::accumulate: (" << x << ", " << y << ") + offset " << offset << " = " << result << std::endl;
        return result;
    }
};

int main() {
    // Define an Action taking two ints and returning int
    Action<int(int, int)> action;

    MyClass obj;

    // 1. Register global function
    action += EVENT_GLOBAL(global_add, global_add, int, (int, int));

    // 2. Register static function
    action += EVENT_STATIC_MEMBER(Math, multiply, Math::multiply, int, (int, int));

    // 3. Register member function
    action += EVENT_MEMBER(MyClass, accumulate, &obj, int, (int, int));

    // 4. Register lambda with capture
    auto lambda = [](int x, int y) {
        int result = x - y;
        std::cout << "lambda: " << x << " - " << y << " = " << result << std::endl;
        return result;
    };
    action += EVENT_LAMBDA_CAPTURE(lambda, int, (int, int));

    std::cout << "-- Invoking all registered callbacks --" << std::endl;
    int final_result = action.invoke(5, 3);
    std::cout << "Final result: " << final_result << std::endl;

    std::cout << "-- Invoking callback by specific key --" << std::endl;
    int acc_result = action.invoke_with_key(GET_KEY_EVENT_MEMBER(MyClass, accumulate, &obj), 2, 4);
    std::cout << "Accumulate result: " << acc_result << std::endl;

    std::cout << "-- Removing global_add callback --" << std::endl;
    action -= GET_KEY_EVENT_GLOBAL(global_add, global_add);
    action.invoke(7, 2);

    std::cout << "-- Clearing all callbacks --" << std::endl;
    action.clear();
    std::cout << "Remaining handler count: " << action.size() << std::endl;

    return 0;
}
```
