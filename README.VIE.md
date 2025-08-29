# Action.hpp

## Tổng quan

`Action.hpp` là một **thư viện header-only, type-safe delegate/event system** cho C++.
Cho phép bạn đăng ký các callback (member function, const member, global/static function, lambda), thực thi chúng và xóa một cách an toàn. Thư viện gọn nhẹ và lấy cảm hứng từ delegate trong C#.

---

## Yêu cầu : C++ 17

---

## Sử dụng bộ nhớ trên hệ thống 64-bit

| Loại callback          | Dung lượng ước tính                                          |
| ---------------------- | ------------------------------------------------------------ |
| Member Function        | 24 bytes (16 heap + 8 ptr trong action::action)                      |
| Static/Global Function | 16 bytes (8 heap + 8 ptr trong action::action)                       |
| Lambda (có capture và không capture)    | 16 (8 heap + 8 ptr trong action::action) + kích thước lambda (align) |

---

## Public API

* `action::action<Signature>` — container chứa các callback.
* `action::action<Signature>::push_bach<Class, &Class::func>(object)` — thêm callback từ member/const member function.
* `action::action<Signature>::push_bach<&GlobalFunc>()` — thêm callback từ global/static function.
* `action::action<Signature>::push_bach<KEY>(lambda)` — thêm callback (có capture và không capture), cần key duy nhất để xóa sau này.
* `action::action<Signature>::erase<Class, &Class::func>(object)` — xóa callback từ member/const member function.
* `action::action<Signature>::erase<&GlobalFunc>()` — xóa callback global/static function.
* `action::action<Signature>::erase<KEY>()` — xóa callback từ key.
* `invoke(args...)` — thực thi tất cả callback đã đăng ký.

---

## Ví dụ sử dụng

```cpp
#include "ActionRemake.hpp"
#include <iostream>

struct MyClass {
    void member(int x) { std::cout << "Thành viên: " << x << "\n"; }
    void member(unsigned long long _double) { std::cout << "Thành viên: " << _double << "\n"; }

    void member_const(int x) const { std::cout << "Thành viên const: " << x << "\n"; }
    static void _static(int x) { std::cout << "Hàm static: " << x << "\n"; }

    void shared_callback(int x) { std::cout << "shared_ptr: " << x << "\n"; }
    void weak_callback(int x) { std::cout << "weak_ptr: " << x << "\n"; }

    void object_callback(int x) { std::cout << "callback từ object: " << x << "\n"; }
};

// Hàm global
void global(int x) { std::cout << "Hàm global: " << x << "\n"; }

int main() {
    MyClass obj;
    const MyClass const_obj;
    action::action<void(int)> onEvent;         // sự kiện không trả về
    action::action<int(int)> onEventReturn;    // sự kiện trả về giá trị int

    // Tạo callback trực tiếp từ object
    auto my_callback = 
        action::object_callback<void(int)>::make_unique<MyClass, &MyClass::object_callback>(&obj);

    // Thêm callback từ member function
    onEvent.push_back<MyClass, &MyClass::member>(&obj);

    // Thêm callback từ const member function
    onEvent.push_back<MyClass, &MyClass::member_const>(&const_obj);

    // Thêm callback từ static / global function
    onEvent.push_back<&MyClass::_static>();
    onEvent.push_back<&global>();

    // Thêm lambda có capture
    int val = 5;
    auto lambda = [val](int x){ std::cout << "Lambda capture: " << x + val << "\n"; };
    onEvent.push_back<42>(lambda);

    // Thêm callback trả về giá trị
    auto global_return = [](int x){ 
        std::cout << "Return: " << x << "\n"; 
        return x*2; 
    };
    onEventReturn.push_back<24>(global_return);

    // Gọi tất cả callback
    std::cout << "Gọi tất cả callback:\n";
    onEvent.invoke(10);

    // Gọi callback trả về giá trị
    std::cout << "\nGọi callback có giá trị trả về:\n";
    int ret = onEventReturn.invoke(10);
    std::cout << "Giá trị trả về: " << ret << "\n";

    // Thêm callback dùng shared_ptr
    auto sp_obj = std::make_shared<MyClass>();
    onEvent.push_back<MyClass, &MyClass::shared_callback>(sp_obj);

    // Thêm callback dùng weak_ptr
    std::weak_ptr<MyClass> wp_obj = sp_obj;
    onEvent.push_back<MyClass, &MyClass::weak_callback>(wp_obj);

    // Xóa các callback theo key
    onEvent.erase<MyClass, &MyClass::member>(&obj);
    onEvent.erase<42>();

    std::cout << "\nGọi sau khi xóa một số callback:\n";
    onEvent.invoke(20);

    // Gọi callback trực tiếp từ object
    my_callback->invoke(30);
    
    return 0;
}
```

---

## Ghi chú & mẹo

1. **Key cho lambda**

   * Mỗi lambda (có capture và không capture) cần **một key duy nhất** để xóa callback sau này.

2. **Thread-safety**

   * `action::action` **không thread-safe**. Nếu dùng trong multi-thread, cần bảo vệ bằng mutex.

3. **Hành vi trả về của callback**

   * Với `action` trả về giá trị, `invoke` trả **giá trị của callback cuối cùng**, các callback trước vẫn được gọi nhưng kết quả bị bỏ qua.

4. **Hỗ trợ nhiều tham số**

   * `action` hỗ trợ bất kỳ signature nào, ví dụ `void(int, double)` hoặc `int(std::string, float)`.
