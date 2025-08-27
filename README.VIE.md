Hướng dẫn sử dụng Action.hpp

``` cpp
#include "Action.hpp"
#include <iostream>

struct MyClass {
    void member(int x) { std::cout << "Member: " << x << "\n"; }
    static void static(int x) { std::cout << "Static: " << x << "\n"; }
};

void global(int x) { std::cout << "Global: " << x << "\n"; }
static void global_static(int x) { std::cout << "Global Static: " << x << "\n"; }

int main() {
    MyClass obj;
    action::action<void(int)> onEvent;

    onEvent += action::make_callback<&MyClass::member>(&obj);
    onEvent += action::make_callback<&MyClass::static>();
    onEvent += action::make_callback<&global>();
    onEvent += action::make_callback<&global_static>();

    auto lambda = [](int x){ std::cout << "Lambda: " << x << "\n"; };
    onEvent += action::make_callback<42>(lambda); // key = 42

    onEvent.invoke(10);

    // Xóa callback
    onEvent -= action::get_key_callback<&MyClass::member>(&obj);
    onEvent -= action::make_callback<&global_static>();
    onEvent -= action::get_key_callback<42>();

    onEvent.invoke(20);
}
```

Toàn bộ hưỡng dẫn trên là do ChatGPT viết, nếu có gì sai sót thì mong bạn bỏ qua chú tôi lười viết quá.
