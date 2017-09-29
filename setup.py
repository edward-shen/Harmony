from distutils.core import setup
from Cython.Build import cythonize
import testa

# ext_options = {"compiler_directives": {"profile": True}, "annotate": True}
setup(
    name = "Hello World App",
    ext_modules = cythonize("testa.pyx") #,**ext_options)
)

testa.fib(2000)