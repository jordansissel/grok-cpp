
#include "foo.h"

template <class T>
Foo<T>::Foo() {
  this->a = 35;
}

template <class T>
Foo<T>::~Foo() { }


template <class T> class Foo;
