
#include <iostream>
#include <fstream>
#include <string>
#include <boost/xpressive/xpressive.hpp>

using namespace std;
using namespace boost::xpressive;

#include "grokpatternset.hpp"
#include "grokregex.hpp"
#include "grokmatch.hpp"

int main(int argc, char **argv) {
  GrokPatternSet<sregex> default_set;

  if (argc < 2) {
    cout << "Usage: $0 <regexp> [field1 field2]" << endl;
    cout << "Example: ./grok '%SYSLOGBASE% .*?%MAC%' MAC" << endl;
    exit(1);
  }

  GrokMatch<sregex> gm;
  GrokRegex<sregex> gre(argv[1]);

  argv += 2;
  argc -= 2;

  default_set.LoadFromFile("patterns");
  gre.AddPatternSet(default_set);

  GrokMatch<sregex>::match_map_type m;

  string str;
  while (getline(cin, str)) {
    bool success;
    //cout << "Line: " << str << endl;
    success = gre.Search(str, gm);
    if (!success)
      continue;

    GrokMatch<sregex>::match_map_type::const_iterator iter;
    m = gm.GetMatches();
    if (argc == 0) {
      for (iter = m.begin(); iter != m.end(); iter++) {
        string key, val;
        key = (*iter).first;
        val = (*iter).second;
        cout << key << " => " << val << endl;
      }
    } else {
      for (int i = 0; i < argc; i++) {
        string key, val;
        key = argv[i];
        val = m[key];
        cout << key << " => " << val << endl;
      }
    }
  }

  return 0;
}
