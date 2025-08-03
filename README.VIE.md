# Action.hpp
# Hệ thống Delegate/Event trong C++ — Hướng Dẫn Sử Dụng

Hệ thống `Action` dưới đây cho phép bạn đăng ký nhiều hàm callback giống như delegate trong C#. Mỗi `Action<Signature>` có thể chứa nhiều `ObjectEvent<Signature>` và gọi chúng tuần tự khi được `invoke`.

---

## Thành phần chính

- `Action<Return(Args...)>`: Quản lý danh sách các callback (kiểu delegate).

- `ObjectEvent<Return(Args...)>`: Giao diện trừu tượng cho các callback.

- `EVENT_*(...)`: Macro tạo callback từ các loại hàm khác nhau (thành viên, toàn cục, lambda...).

- `GET_KEY_EVENT_*(...)`: Macro tạo khóa (key) để so sánh hoặc xóa callback.

## Các biến trong Macro
- CLASS : là class chứa callback bạn muốn đăng kí
- FUNC : là tên callback mà bạn muốn đăng kí
- PTR : là con trỏ của class (cái này chỉ có ở callback member)
- CALL : là cách để gọi callback (cái này chỉ có ở callback static và global)
- RET : là kiểu trả về của callback
- SIG : là chữ ký của callback
---

## Tính năng
- API delegate/event giống C#
- Hỗ trợ lambda, hàm thành viên, hàm tĩnh và hàm toàn cục
- Cho phép tra cứu hoặc xóa callback theo tên/khoá (`KeyEvent`)
- Mô hình bộ nhớ nhẹ, đơn giản

---

## Bộ nhớ sử dụng trên hệ thống 64-bit

| Loại Callback        | Bộ nhớ ước tính            |
|----------------------|----------------------------|
| Hàm thành viên       | 24 byte (16 heap + 8 trong Action) |
| Hàm tĩnh/toàn cục    | 16 byte (8 heap + 8 trong Action)  |
| Lambda (capture)     | 16 + kích thước lambda     |

---

## Cách sử dụng

### 1. Định nghĩa class sử dụng `Action`:

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
    action += CALLBACK_GLOBAL(global_add, global_add, int, (int, int));

    // 2. Static member function handler
    action += CALLBACK_STATIC_MEMBER(Math, multiply, Math::multiply, int, (int, int));

    // 3. Member function handler
    action += CALLBACK_MEMBER(MyClass, accumulate, &obj, int, (int, int));

    // 4. Lambda handler capture
    auto lambda = [](int x, int y) {
        int result = x - y;
        std::cout << "lambda: " << x << " - " << y << " = " << result << std::endl;
        return result;
    };
    action += CALLBACK_LAMBDA_LOCAL(lambda, int, (int, int));

    std::cout << "-- Invoking all handlers --" << std::endl;
    int final_result = action.invoke(5, 3);
    std::cout << "Final (last) result: " << final_result << std::endl;

    std::cout << "-- Invoke specific handler by key --" << std::endl;
    // Invoke only the accumulate member function
    int acc_result = action.invoke_with_key(GET_KEY_CALLBACK_MEMBER(MyClass, accumulate, &obj), 2, 4);
    std::cout << "Accumulate result: " << acc_result << std::endl;

    std::cout << "-- Removing global_add handler --" << std::endl;
    action -= GET_KEY_CALLBACK_GLOBAL(global_add, global_add);
    action.invoke(7, 2);

    std::cout << "-- Clearing all handlers --" << std::endl;
    action.clear();
    std::cout << "Handler count after clear: " << action.size() << std::endl;

    return 0;
}
```
---
Toàn bộ hưỡng dẫn trên là do ChatGPT viết, nếu có gì sai sót thì mong bạn bỏ qua chú tôi lười viết quá.
