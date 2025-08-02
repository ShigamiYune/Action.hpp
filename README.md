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

class Myclass { 
  Action<int(int, int)> test_1; 
  Action<int(int, int)> test_2; 
  
  DEFINE_EVENT(test, Myclass) 
  DEFINE_EVENT(test2, Myclass) 
  
  void init() { 
    test_1 += MAKE_EVENT(test, Myclass, int(int, int)); 
    test_2 += MAKE_EVENT(test2, Myclass, int(int, int)); 
  } 
  
  int test(int i, int j) { 
    return i; 
  } 

  int test2(int i, int j) { 
    return j; 
  }
};

int main(){
  MyClass my_class;
  my_class.init();

  my_class.test_1.invoke(0, 1);
  my_class.test_2.invoke(1, 0);

  return 0;
}
```
