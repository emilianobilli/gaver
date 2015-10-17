from distutils.core import setup, Extension
setup(name="pylibgv", version="0.1",
      ext_modules=[
      Extension("pylibgv", ["pylibgv.c"],include_dirs=['./', '/usr/include/python2.6/'],libraries=['gv']),
                  ])