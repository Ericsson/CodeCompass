import sys
import os
import importlib.util
import traceback
from jedi.api.classes import Name
from parserlog import log, bcolors
from parserconfig import ParserConfig

class PYBuiltin:
    builtin = {}

    @staticmethod
    def findBuiltins(config: ParserConfig):
        try:
            # Note: Python 3.10+ required
            stdlib_modules = sys.stdlib_module_names

            for e in stdlib_modules:
                spec = importlib.util.find_spec(e)
                if spec and spec.origin:
                    PYBuiltin.builtin[spec.origin] = True

                if spec and spec.submodule_search_locations:
                    for submodule_dir in spec.submodule_search_locations:
                        for root, dirs, files in os.walk(submodule_dir):
                            for file in files:
                                p = os.path.join(root, file)
                                ext = os.path.splitext(p)[1]

                                if ext and ext.lower() == '.py':
                                    PYBuiltin.builtin[p] = True

        except:
            log(f"{bcolors.FAIL}Failed to find Python builtins!")
            if config.stack_trace:
                traceback.print_exc()

    @staticmethod
    def isBuiltin(name: Name):
        path = str(name.module_path)
        return (path in PYBuiltin.builtin or 
                name.in_builtin_module() or
                "/typeshed/stdlib/" in path)

