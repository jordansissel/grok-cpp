PACKAGE=cgrok

CFLAGS=-pipe -I/usr/local/include

CFLAGS=-Os -static
#CFLAGS+=-g
#CFLAGS+=--param ggc-min-expand=100 --param ggc-min-heapsize=90
#CFLAGS+=-O2

LDFLAGS=-L/usr/local/lib -lpopt

all: grok

grok: main.o
	$(CXX) $(LDFLAGS) main.o -o $@

pattern_test.o: fileobserver.hpp grokconfig.hpp grokfile.hpp grokmatch.hpp grokpattern.hpp grokpatternset.hpp grokpredicate.hpp grokregex.hpp watchfileentry.hpp
pattest: pattern_test.o
	$(CXX) pattern_test.o -o $@

patfind: pattern_discovery.o
	$(CXX) pattern_discovery.o -o $@

filetest: filetest.o
	$(CXX) filetest.o -o $@

clean:
	rm *.o grokre test test_patterns patfind grok filetest > /dev/null 2>&1 || true

filetest.o: filetest.cpp fileobserver.hpp
pattern_discovery.o: grokpatternset.hpp grokregex.hpp grokmatch.hpp grokpredicate.hpp
main.o: grokpatternset.hpp grokregex.hpp grokmatch.hpp grokpredicate.hpp grokconfig.hpp fileobserver.hpp
grokpatternset.hpp: grokpattern.hpp

.cpp.o:
	$(CXX) $(CFLAGS) -c -o $@ $<

.cc.o:
	$(CXX) $(CFLAGS) -c -o $@ $<

package: build-package test-package

test:
	make -C t test

build-package:
	#@svn status -q | wc -l | awk '$$1 > 0 { print "svn changes are not checked in!"; exit(1) }'
	PACKAGE=$(PACKAGE) sh release.sh

test-package: build-package
	PKGVER=$(PACKAGE)-`date "+%Y%m%d"`; \
	tar -C /tmp -xf $${PKGVER}.tar.gz; \
	echo "Running C++ tests..." && make -C /tmp/$${PKGVER}/t test; \
	echo "Running Python tests..." && make -C /tmp/$${PKGVER}/pygrok test;
