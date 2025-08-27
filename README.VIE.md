# Action.hpp

## Tổng quan

`Action.hpp` là một **thư viện header-only, type-safe delegate/event system** cho C++.
Cho phép bạn đăng ký các callback (member function, const member, global/static function, lambda), thực thi chúng và xóa một cách an toàn. Thư viện gọn nhẹ và lấy cảm hứng từ delegate trong C#.

---

## Yêu cầu : C++ 11

---

## Sử dụng bộ nhớ trên hệ thống 64-bit

| Loại callback          | Dung lượng ước tính                                          |
| ---------------------- | ------------------------------------------------------------ |
| Member Function        | 24 bytes (16 heap + 8 ptr trong Action)                      |
| Static/Global Function | 16 bytes (8 heap + 8 ptr trong Action)                       |
| Lambda (có capture)    | 16 (8 heap + 8 ptr trong Action) + kích thước lambda (align) |

---

## Public API

* `action::action<Signature>` — container chứa các callback.
* `action::make_callback<&Class::func>(object)` — tạo callback từ member/const member function.
* `action::make_callback<&GlobalFunc>()` — tạo callback từ global/static function.
* `action::make_callback<KEY>(lambda)` — tạo callback lambda có capture, cần key duy nhất để xóa sau này.
* `action::get_key_callback<&Class::func>(object)` — lấy key để xóa callback.
* `action::get_key_callback<&GlobalFunc>()` — lấy key để xóa callback.
* `action::get_key_callback<KEY>()` — lấy key để xóa callback.
* `operator+=` — thêm callback vào `action`.
* `operator-=` — xóa callback bằng key.
* `invoke(args...)` — thực thi tất cả callback đã đăng ký.

---

## Ví dụ sử dụng

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

    // Thêm member callback
    onEvent += action::make_callback<&MyClass::member>(&obj);

    // Thêm const member callback
    onEvent += action::make_callback<&MyClass::member_const>(&const_obj);

    // Thêm static/global callbacks
    onEvent += action::make_callback<&MyClass::_static>();
    onEvent += action::make_callback<&global>();

    // Thêm lambda có capture
    int val = 5;
    auto lambda = [val](int x){ std::cout << "Lambda capture: " << x + val << "\n"; };
    onEvent += action::make_callback<42>(lambda);

    // Thêm callback trả về giá trị
    auto global_return = [](int x){ std::cout << "Return: " << x << "\n"; return x*2; };
    onEventReturn += action::make_callback<24>(global_return);

    // Thực thi tất cả callback
    std::cout << "Thực thi tất cả callback:\n";
    onEvent.invoke(10);

    // Thực thi callback trả về
    std::cout << "\nThực thi callback trả về:\n";
    int ret = onEventReturn.invoke(10);
    std::cout << "Giá trị trả về: " << ret << "\n";

    // Xóa callback bằng key
    onEvent -= action::get_key_callback<&MyClass::member>(&obj);
    onEvent -= action::get_key_callback<42>();

    std::cout << "\nThực thi sau khi xóa một số callback:\n";
    onEvent.invoke(20);

    return 0;
}
```

---

## Ghi chú & mẹo

1. **Key cho lambda**

   * Mỗi lambda có capture cần **một key duy nhất** để xóa callback sau này.

2. **Member function overload**

   * Template không tự phân biệt overload; nếu có overload, cần **cast rõ ràng** hoặc đổi tên function.

3. **Thread-safety**

   * `action::action` **không thread-safe**. Nếu dùng trong multi-thread, cần bảo vệ bằng mutex.

4. **Hành vi trả về của callback**

   * Với `action` trả về giá trị, `invoke` trả **giá trị của callback cuối cùng**, các callback trước vẫn được gọi nhưng kết quả bị bỏ qua.

5. **Hỗ trợ nhiều tham số**

   * `action` hỗ trợ bất kỳ signature nào, ví dụ `void(int, double)` hoặc `int(std::string, float)`.

---

## Flow lifecycle của callback

```
+-------------------+
| Tạo Callback       |
| make_callback()    |
+-------------------+
          |
          v
+-------------------+
| Thêm vào Action    |
| operator+=         |
+-------------------+
          |
          v
+-------------------+
| Thực thi Callback  |
| invoke(args...)    |
+-------------------+
          |
          v
+-------------------+
| Xóa Callback       |
| operator-= / key   |
+-------------------+
```
