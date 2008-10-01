
CXX=g++
CFLAGS=-I/usr/local/include

grokre: grokre.o grokmatch.o grokregex.o
	$(CXX) -L/usr/local/lib -lonig grokre.o grokmatch.o grokregex.o -o $@

xpressivetest: boost_xpressive_test.o
	$(CXX) $(LDFLAGS) boost_xpressive_test.o -o $@

clean:
	rm *.o grokre > /dev/null 2>&1 || true

.cpp.o:
	$(CXX) $(CFLAGS) -c -o $@ $<

.cc.o:
	$(CXX) $(CFLAGS) -c -o $@ $<

