
_grok.so: grok_wrap.o
	g++ -shared grok_wrap.o -o _grok.so

_grok_boost.so: grok_boost_wrap.cpp
	g++ -fPIC -I/usr/include/python2.5 -o _grok_boost.so -shared grok_boost_wrap.cpp

grok_wrap.o: grok.swig
	swig -c++ -I/usr/include -python -o grok_wrap.cpp grok.swig
	g++ -fPIC -I/usr/include -c grok_wrap.cpp -o grok_wrap.o -I/usr/include/python2.5/

test: _grok.so
	python -c 'import grok; print dir(grok)'

clean: 
	rm *.o *.so *.pyc grok.py
