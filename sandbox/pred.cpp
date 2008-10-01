#include "grokpredicate.hpp"

int main(int argc, char **argv) {
  string pred = argv[1];
  GrokPredicate<sregex> foo(pred);
  sregex re = sregex::compile(".*") [ check(foo) ];
  smatch m;

  string data = argv[2];
  regex_search(data, m, re);
  cout << "Match on '" << data << "' with re /.*/" << endl;
  cout << "Predicate: " << pred << endl;

  cout << "Result: " << m.str(0) << endl;



  return 0;
}
