#include <map>
#include <string>
#include <boost/xpressive/xpressive.hpp>

using namespace std;
using namespace boost::xpressive;

#include "patterns.h"

/* XXX: Maybe we should worry about using cregex/sregex/wsregex etc? */


//static sregex_map_t default_patterns;
static GrokPatternSet default_patterns;
static void init();

pattern_map_t& DefaultPatterns() {
  if (default_patterns.size() == 0)
    init();
  return default_patterns;
}

void init() {
  //default_patterns["WORD"] = _b >> +_w >> _b;
  //default_patterns["WORD"] = pattern::compile("\\w+");

  default_patterns["WORD"] = "\\w+";
  default_patterns["NUMBER"] = "(?:[+-]?(?:(?:[0-9]+(?:\\.[0-9]*)?)|(?:\\.[0-9]+)))";
}
