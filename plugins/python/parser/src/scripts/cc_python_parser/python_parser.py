import os
from pathlib import PurePath

from cc_python_parser.parse_exception import ParseException
from cc_python_parser.parser import Parser
from cc_python_parser.persistence.persistence import init_persistence, ModelPersistence


def parse(source_root: str, persistence):
    init_persistence(persistence)

    def directory_exception(path: PurePath) -> bool:
        directory = os.path.basename(os.path.normpath(str(path)))
        return directory.startswith('.') or directory == 'venv'

    def file_exception(path: PurePath) -> bool:
        return False

    exception = ParseException(directory_exception, file_exception)
    p = Parser([source_root], ModelPersistence(persistence), exception)
    p.parse()
    pass


if __name__ == '__main__':
    parse("dummy", None)   # error!
