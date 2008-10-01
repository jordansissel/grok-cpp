#!/usr/bin/env python

import sys
import re
from pygrok import pygrok
from pygrok import patterns

PUNCT_RE = re.compile(r"""['":_.=+-?]""")
SKIP_PATTERNS = ("DATA GREEDYDATA USER USERNAME WORD NOTSPACE PID PROG "
    "QS QUOTEDSTRING YEAR URIHOST URIPARAM URIPATH URIPATHPARAM").split()

def re_weight(gre_str):
  score = len(gre_str)
  score += 1.5 * (gre_str.count(" ") + gre_str.count(r"\s"))
  score += 1 + gre_str.count(".")
  score += .6 * len(PUNCT_RE.findall(gre_str))
  return score

def GetSortedPatterns():
  expansion = {}
  gre = pygrok.GrokRegex()
  gpatterns = patterns.base_patterns
  gre.add_patterns(gpatterns)
  for name in gpatterns:
    gre.set_regex("%%%s%%" % name)
    expansion[name] = gre.get_expanded_regex()

  pat_list = sorted(expansion.items(), key=lambda (k,v): re_weight(v))
  pat_list.reverse()
  gre_list = []

  for (name, unused_expanded_re) in pat_list:
    gre = pygrok.GrokRegex()
    gre.add_patterns(gpatterns)
    gre.set_regex(r"%%%s=~\W%%" % (name))
    gre_list.append( (name, gre) )

  return gre_list

def PrunePatterns(gre_list):
  new_list = gre_list[:]
  for i in gre_list:
    if i[0] in SKIP_PATTERNS:
      new_list.remove(i)
  return new_list

def Analyze(line, gre_list):
  for name, gre in gre_list:
    done = False

    # We exit this loop when we no longer match the given pattern
    while not done:
      m = gre.search(line)
      if not m:
        done = True
        continue
      pos = int(m["=POSITION"])
      len = int(m["=LENGTH"])
      repl = "%%%s%%" % name
      line = line[0:pos] + repl + line[pos+len:]
  return line

def main():
  gre_list = GetSortedPatterns()
  gre_list = PrunePatterns(gre_list)

  #for i in gre_list:
    #print i[0]

  for line in sys.stdin:
    print Analyze(line[:-1], gre_list)

if __name__ == "__main__":
  main()
