#!/usr/bin/env python

import sys
import re
from pygrok import pygrok
from pygrok import patterns


mypat = {
  #'UNIXPATH': r'''(?<![\w\/])(?:/(?:[\w_@:.,-]*|\.)+)''',
  'UNIXPATH': r'''(?<![\w\\/])(?:/(?:[\w_@:.,-]+|\\.)*)+''',
}

gre = pygrok.GrokRegex()
#gre.add_patterns(mypat)
gre.add_patterns(patterns.base_patterns)
gre.set_regex(sys.argv[1])
print gre.get_expanded_regex()
data = sys.stdin
#data = ["/foo", r"/bar\ baz", r"foo\ bar"]
for i in data:
  m = gre.search(i)
  if m:
    print m
