#include <iostream>
using namespace std;

template <class foo>
class Foo {
  public:
    Foo();
    ~Foo();
 
    foo a;
};

template <class foo>
Foo<foo>::Foo() {
  a = 30;
}

template <class foo>
Foo<foo>::~Foo() {
  /* nothing */
}

int main() {
  Foo<int> f;

  cout << f.a << endl;
}
