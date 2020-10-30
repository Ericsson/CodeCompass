import ast
import os
from pathlib import PurePath
from typing import Optional

from my_ast.common.parser_tree import ParserTree
from my_ast.file_info import FileInfo
from my_ast.import_finder import ImportFinder
from my_ast.parse_exception import ParseException
from my_ast.parser import Parser
from my_ast.symbol_collector import SymbolCollector


def main():
    # with open("../test_dir/test_file_2.py", "r") as source:
    #     tree = ast.parse(source.read())
    #
    # pt = ParserTree(tree)
    #
    # fi = FileInfo("test_file_2.py", PurePath("../test_dir/test_file_2.py"))
    #
    # class ImportFinderHelper(ImportFinder):
    #     def get_file_by_location(self, location: PurePath) -> Optional:
    #         return None
    #
    # vdc = SymbolCollector(pt, fi.preprocessed_file, ImportFinderHelper())
    # vdc.collect_symbols()

    def directory_exception(path: PurePath) -> bool:
        directory = os.path.basename(os.path.normpath(str(path)))
        return directory.startswith('.') or directory == 'venv'

    def file_exception(path: PurePath) -> bool:
        return False

    exception = ParseException(directory_exception, file_exception)
    p = Parser(["F:/ELTE/PythonPlugin"], exception)
    p.parse()
    pass


if __name__ == '__main__':
    main()
