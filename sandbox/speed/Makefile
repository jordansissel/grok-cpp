
xpressive: xpressive.cpp
	g++ $< -o $@

xpressive2: xpressive.cpp
	g++ -O2 $< -o $@

xpressive3: xpressive.cpp
	g++ -O3 $< -o $@

xpressive_testgroup: xpressive_testgroup.cpp
	g++ $< -o $@

xpressive2_testgroup: xpressive_testgroup.cpp
	g++ -O2 $< -o $@

xpressive3_testgroup: xpressive_testgroup.cpp
	g++ -O3 $< -o $@

xpressive2_testmatch: xpressive_testmatch.cpp
	g++ -O2 $< -o $@

ITERATIONS=3000000
test_plain: xpressive2 
	time ./xpressive2 $(ITERATIONS)
	time perl matchip.pl $(ITERATIONS)

test_group: xpressive2_testgroup
	time ./xpressive2_testgroup $(ITERATIONS)
	time perl matchip_testgroup.pl $(ITERATIONS)

test_match: xpressive2_testmatch
	time ./xpressive2_testmatch $(ITERATIONS)
	time perl matchip_testmatch.pl $(ITERATIONS)
