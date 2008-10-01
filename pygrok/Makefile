
all: 
	# Force CXX=g++ here in case we are using ccache. distutils sucks and turns
	# 'CXX=ccache g++" into just "ccache". Sigh...
	CXX=g++ python setup.py build

install: pygrok/patterns.py
	python setup.py install

test: all
	make -C t test

clean:
	rm build/*/pygrok/*.so || true
	rm build/*/pygrok/*.o || true

pygrok/patterns.py: ../patterns
	python convpatterns.py < ../patterns > $@
