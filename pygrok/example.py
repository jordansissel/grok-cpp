#!/usr/bin/env python

import sys
import os

base_dir = "build"
for i in os.listdir(base_dir):
  if i.startswith("lib."):
    sys.path.append("%s/%s" % (base_dir, i))
   
import pygrok

patterns = {}
for i in open("../patterns"):
  i = i[:-1]
  if not i or i.startswith("#"):
    continue
  (name, pattern) = i.split(" ", 1)
  patterns[name] = pattern

g = pygrok.GrokRegex()
g.add_patterns(patterns)

matchpattern = sys.argv[1]
g.set_regex(matchpattern)

for i in sys.stdin:
  m = g.search(i[:-1])
  if m:
    print m

