#!/bin/sh


for pattern in `awk '/^[^#]/ { print $1 }' < ../patterns`; do
  echo "void testPattern${pattern}() {"
  echo '  string s = "'${pattern}'";'
  echo '  _testPatternMatch(s, true);'
  echo '}'

  echo "void testPattern${pattern}Fails() {"
  echo '  string s = "'${pattern}'";'
  echo '  _testPatternMatch(s, false);'
  echo '}'
done \
| sed -e 's/^/    /'
