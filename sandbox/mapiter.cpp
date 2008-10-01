

#include <map>
#include <string>
#include <iostream>

using namespace std;

int main() {
  map<string, string> m;

  m["Hello"] = "testing";

  map<string, string>::iterator iter;

  for (iter = m.begin(); iter != m.end(); iter++) {
    cout << (*iter).first << ": " << (*iter).second << endl;
  }

  return 0;
}
