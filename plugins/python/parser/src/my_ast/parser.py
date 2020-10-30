import ast
import os
import sys
from pathlib import PurePath
from typing import List, Optional, Union

from my_ast.common.parser_tree import ParserTree
from my_ast.file_info import FileInfo, ProcessStatus
from my_ast.function_symbol_collector import FunctionSymbolCollector
from my_ast.function_symbol_collector_factory import FunctionSymbolCollectorFactory
from my_ast.import_finder import ImportFinder
from my_ast.logger import logger, db_logger
from my_ast.parse_exception import DefaultParseException, ParseException
from my_ast.persistence.persistence import model_persistence
from my_ast.scope import GlobalScope
from my_ast.symbol_collector import SymbolCollector

# TODO: builtins - dir(builtins)
# TODO: change logging to DB storing
from my_ast.symbol_collector_interface import IFunctionSymbolCollector
from my_ast.type_data import DeclarationType

current_file = ""  # TODO: only for debug, remove it


class Parser(ast.NodeVisitor, ImportFinder, FunctionSymbolCollectorFactory):
    def __init__(self, directories: List[str], exceptions: Optional[ParseException] = None):
        self.directories: List[str] = directories
        if exceptions is None:
            exceptions = DefaultParseException()
        self.exceptions: ParseException = exceptions
        self.files: List[FileInfo] = []
        self.parsing_started_files = []
        self.collect_files()

    def collect_files(self):
        for directory in self.directories:
            sys.path.append(directory)
            self.process_directory(PurePath(directory))

    def process_directory(self, directory: PurePath):
        sub_directories = []
        for element in os.listdir(str(directory)):
            path = directory.joinpath(element)
            if os.path.isfile(str(path)):
                if self.exceptions.is_file_exception(path):
                    continue
                if element.endswith(".py"):
                    self.files.append(FileInfo(element, path))
            elif os.path.isdir(str(path)):
                if self.exceptions.is_dictionary_exception(path):
                    continue
                sub_directories.append(path)
            else:
                assert False, "Unknown element (file, directory): " + str(path)

        for subdir in sub_directories:
            self.process_directory(subdir)

    def parse(self) -> None:
        for file_info in self.files:
            if file_info.symbol_collector is None:
                self.parse_file(file_info)
        self.persist_global_scopes()

    def parse_file(self, file_info: FileInfo) -> None:
        global current_file
        current_file = file_info.file
        if file_info.status != ProcessStatus.WAITING:
            return
        # if file_info in self.parsing_started_files:
        #     return
        # self.parsing_started_files.append(file_info)
        logger.debug('\nFILE: ' + file_info.file[:-3] + '\n=========================================')
        db_logger.debug('\nFILE: ' + file_info.file[:-3] + '\n=========================================')
        # TODO: fix this...
        # if file_info.file[:-3] in builtin_module_names:  # [:-3] - '.py' extension
        #     return  # TODO: handle this case (+ deprecated modules!)
        if file_info.path is None:
            return
        try:
            with open(str(file_info.path), "r") as source:
                tree = ast.parse(source.read())
            file_info.preprocess_file(tree)
            for dependency in file_info.preprocessed_file.import_table.get_dependencies():
                dependency_file_info = [x for x in self.files if x.path == dependency.location]
                if len(dependency_file_info) == 0:
                    pass
                    # TODO: do we need this? (eg. PYTHONPATH)
                    # new_file_info = FileInfo(file, dependency.location)
                    # self.files.append(new_file_info)
                    # self.parse_file(new_file_info)
                    # logger.debug('\nFILE: ' + file_info.file[:-3] + '\n=========================================')
                elif len(dependency_file_info) == 1:
                    if dependency_file_info[0].status is ProcessStatus.WAITING:
                        self.parse_file(dependency_file_info[0])
                        current_file = file_info.file
                        logger.debug('\nFILE: ' + file_info.file[:-3] + '\n=========================================')
                        db_logger. \
                            debug('\nFILE: ' + file_info.file[:-3] + '\n=========================================')
                else:
                    assert False, 'Multiple file occurrence: ' + dependency.get_file()
            sc = SymbolCollector(ParserTree(tree), file_info.path, file_info.preprocessed_file, self, self)
            sc.collect_symbols()
            file_info.set_variable_collector(sc)

            if model_persistence is not None:
                model_persistence.persist_file(file_info.create_dto())
        except FileNotFoundError:
            return

    def get_file_by_location(self, location: PurePath) -> Optional[FileInfo]:
        for file in self.files:
            if file.path == location:
                return file
        return None

    def persist_global_scopes(self):
        for file_info in self.files:
            if file_info.symbol_collector is not None:
                assert isinstance(file_info.symbol_collector.scope_manager.get_current_scope(), GlobalScope)
                file_info.symbol_collector.scope_manager.persist_current_scope()

    def get_function_symbol_collector(self, symbol_collector: SymbolCollector,
                                      func_def: Union[ast.FunctionDef, ast.AsyncFunctionDef],
                                      arguments: List[DeclarationType]) -> IFunctionSymbolCollector:
        return FunctionSymbolCollector(symbol_collector, func_def, arguments)
