
import sys

print "base_patterns = {"
for i in sys.stdin:
  i = i[:-1]
  if not i or i.startswith("#"):
    continue
  (name, pattern) = i.split(None, 1)

  print "  '%s': r'''%s'''," % (name, pattern)

print "}"
