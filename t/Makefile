
CXXGEN=cxxtest/cxxtestgen.pl

CFLAGS=-pipe -I/usr/local/include -I./cxxtest -I../
#CFLAGS=-g
CFLAGS+=-Os

.SUFFIXES: .test .cpp .o

basic: basic.o
	$(CXX) $(LDFLAGS) $< -o $@
	strip $@

patterns: patterns.o
	$(CXX) $(LDFLAGS) $< -o $@
	strip $@

patterns.test: patterns.test_template pattern_test_set.generated_test
	rm -f $@
	perl -ne 'if (/%PATTERN_TESTS%/) { print `cat pattern_test_set.generated_test`; } else { print $$_ }' < patterns.test_template > $@
	chmod 444 $@

pattern_test_set.generated_test: ../patterns generate_pattern_test.sh
	sh generate_pattern_test.sh > $@

test: basic patterns
	./basic
	./patterns

clean:
	rm -f *.o || true
	rm basic patterns || true

.cpp.o:
	$(CXX) $(CFLAGS) -c -o $@ $<

.test.cpp:
	$(CXXGEN) --error-printer -o $@ $<
