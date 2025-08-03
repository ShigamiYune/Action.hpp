# Action.hpp
# 🔧 C++ Delegate/Event System — How to Use

The `Action` system below allows you to register multiple callbacks as delegates similar to events in C#. Each `Action<Signature>` can contain multiple `ObjectEvent<Signature>`, and call them sequentially when `invoked`.

---

## 📦 Main components

- `Action<Return(Args...)>`: Manages a list of events (delegate type).

- `ObjectEvent<Return(Args...)>`: Abstract interface for callbacks.

- `DEFINE_EVENT(...)`: Macro defines a callback adapter for a specific class.

- `MAKE_EVENT(...)`: Macro creates a callback instance from the current object.

---

## 🛠 How to use

### 1. Define class using events:

```cpp
#include "Action.hpp"

// Global free function
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

// A sample class with member function to handle events
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
    Action<int(int,int)> action;

    MyClass obj;

    // 1. Global function handler
    action += EVENT_GLOBAL(global_add, global_add, int, (int, int));

    // 2. Static member function handler
    action += EVENT_STATIC_MEMBER(Math, multiply, Math::multiply, int, (int, int));

    // 3. Member function handler
    action += EVENT_MEMBER(MyClass, accumulate, &obj, int, (int, int));

    // 4. Lambda handler capture
    auto lambda = [](int x, int y) {
        int result = x - y;
        std::cout << "lambda: " << x << " - " << y << " = " << result << std::endl;
        return result;
    };
    action += EVENT_LAMBDA_CAPTURE(lambda, int, (int, int));

    std::cout << "-- Invoking all handlers --" << std::endl;
    int final_result = action.invoke(5, 3);
    std::cout << "Final (last) result: " << final_result << std::endl;

    std::cout << "-- Invoke specific handler by key --" << std::endl;
    // Invoke only the accumulate member function
    int acc_result = action.invoke_with_key(GET_KEY_EVENT_MEMBER(MyClass, accumulate, &obj), 2, 4);
    std::cout << "Accumulate result: " << acc_result << std::endl;

    std::cout << "-- Removing global_add handler --" << std::endl;
    action -= GET_KEY_EVENT_GLOBAL(global_add, global_add);
    action.invoke(7, 2);

    std::cout << "-- Clearing all handlers --" << std::endl;
    action.clear();
    std::cout << "Handler count after clear: " << action.size() << std::endl;

    return 0;
}
```
