#ifndef __STRING_UTILS
#include <iostream>
#include <string>

namespace StringUtils {
  using namespace std;
  void Unescape(string &value) {
    string new_value(value);
    int last_pos = -1;
    int offset = 0;

    while ((last_pos = (int)value.find("\\", last_pos + 1)) != string::npos) {
      string repl("");
      int len = 1;
      if (value[last_pos + 1] == '%') {
        repl = "%";
        len += 1;
      }

      if (len > 1) {
        new_value.replace(last_pos - offset, len, repl);
        offset = offset + (len - repl.size());
      }
    }

    value = new_value;
  }
};

#endif /* ifdef __STRING_UTILS */
