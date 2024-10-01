import sys
from jedi.api.classes import Name
from parserlog import log, bcolors

class PYBuiltin:
    builtin = {}

    @staticmethod
    def findBuiltins():
        try:
            # Note: Python 3.10+ required
            stdlib_modules = sys.stdlib_module_names
            for key, val in sys.modules.items():
                if hasattr(val, "__file__") and key in stdlib_modules:
                    PYBuiltin.builtin[val.__file__] = True
        except:
            log(f"{bcolors.FAIL}Failed to find Python builtins!")

    @staticmethod
    def isBuiltin(name: Name):
        path = str(name.module_path)
        return (path in PYBuiltin.builtin or 
                name.in_builtin_module() or
                "/typeshed/stdlib/" in path)

