#include <string>
#include <iostream>
#include <stdio.h>

using namespace std;

int main() {
  string foo;

  foo = "hello there";
  #cout << foo << endl;
  printf("%s\n", foo.c_str());
  return 0;
}
