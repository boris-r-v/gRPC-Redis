#include <stdio.h>

namespace B {
    struct B {};
    void call(void (*f)()) {
        f();
    }
}

template<class T>
void f() {
    puts("Hello world");
}

int main() {
    call(f<B::B>);
}

