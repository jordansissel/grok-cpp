#!/usr/bin/env python

import sys
import os

base_dir = "../build"
for i in os.listdir(base_dir):
  if i.startswith("lib."):
    sys.path.insert(0, "%s/%s" % (base_dir, i))
   
from pygrok import patterns
from pygrok import pygrok

g = pygrok.GrokRegex()
g.add_patterns(patterns.base_patterns)

g.set_regex("Hello %WORD%")
m = g.search("Hello world")
assert m
assert m["WORD"] == "world"
