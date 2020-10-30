import os
from pathlib import PurePath

from my_ast.parse_exception import ParseException
from my_ast.parser import Parser
from my_ast.persistence.persistence import init_persistence


def parse(source_root: str, persistence):
    print(source_root)

    print(persistence.f())

    init_persistence(persistence)

    def directory_exception(path: PurePath) -> bool:
        directory = os.path.basename(os.path.normpath(str(path)))
        return directory.startswith('.') or directory == 'venv'

    def file_exception(path: PurePath) -> bool:
        return False

    exception = ParseException(directory_exception, file_exception)
    p = Parser([source_root], exception)
    p.parse()
    pass


if __name__ == '__main__':
    parse("dummy", None)   # error!
