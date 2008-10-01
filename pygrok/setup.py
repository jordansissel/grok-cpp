from distutils.core import setup, Extension
setup(name="pygrok",
      version="1.0",
      description = "Python bindings to C++ Grok",
      url = "http://www.semicomplete.com/projects/grok",
      author = "Jordan Sissel",

      packages = [
        "pygrok",
      ],

      ext_modules=[Extension("pygrok.pygrok", ["pygrok/pygrok.cpp"])],
      include_dirs=["/usr/local/include"],
    )
