import ast
import os
import sys
from pathlib import PurePath, Path
from typing import List, Optional, Union, Set, Tuple

from cc_python_parser.common.parser_tree import ParserTree
from cc_python_parser.common.utils import process_file_content
from cc_python_parser.file_info import FileInfo, ProcessStatus
from cc_python_parser.function_symbol_collector import FunctionSymbolCollector
from cc_python_parser.function_symbol_collector_factory import FunctionSymbolCollectorFactory
from cc_python_parser.import_finder import ImportFinder
from cc_python_parser.logger import logger, db_logger
from cc_python_parser.parse_exception import DefaultParseException, ParseException
from cc_python_parser.persistence.persistence import ModelPersistence
from cc_python_parser.scope import GlobalScope
from cc_python_parser.symbol_collector import SymbolCollector
from cc_python_parser.built_in_types import get_all_built_in_types
from cc_python_parser.built_in_functions import get_all_built_in_functions
from cc_python_parser.common.metrics import metrics

# TODO: builtins - dir(builtins)
# TODO: change logging to DB storing
from cc_python_parser.symbol_collector_interface import IFunctionSymbolCollector
from cc_python_parser.type_data import DeclarationType

current_file = ""  # TODO: only for debug, remove it


class Parser(ast.NodeVisitor, ImportFinder, FunctionSymbolCollectorFactory):
    def __init__(self, directories: List[str], persistence: ModelPersistence,
                 exceptions: Optional[ParseException] = None):
        self.directories: List[str] = directories
        self.other_modules: Set[PurePath] = set()
        self.persistence: ModelPersistence = persistence
        if exceptions is None:
            exceptions = DefaultParseException()
        self.exceptions: ParseException = exceptions
        self.files: List[FileInfo] = []
        self.other_module_files: List[FileInfo] = []
        self.parsing_started_files = []
        self.scope_managers = []
        self.collect_files()

    def collect_files(self):
        for directory in self.directories:
            sys.path.append(directory)
            self.process_directory(PurePath(directory))

    def process_directory(self, directory: PurePath):
        for root, _, files in os.walk(directory, followlinks=True):
            for file in files:
                file_path = Path(root, file)
                if file_path.is_symlink():
                    if not file_path.exists():
                        continue
                    file_path = file_path.resolve()
                if file_path.suffix == '.py':
                    self.files.append(FileInfo(file_path))

    def parse(self) -> None:
        metrics.start_parsing()
        self.persist_builtins()
        for file_info in self.files:
            if file_info.symbol_collector is None:
                self.parse_file(file_info)
        self.persist_global_scopes()
        metrics.stop_parsing()

    def parse_file(self, file_info: FileInfo) -> None:
        global current_file
        current_file = file_info.get_file()
        if file_info.status != ProcessStatus.WAITING or current_file[-3:] != '.py':
            return
        logger.debug('\nFILE: ' + file_info.get_file_name() + '\n=========================================')
        db_logger.debug('\nFILE: ' + file_info.get_file_name() + '\n=========================================')
        if file_info.path is None:
            return

        tree = None

        def handle_file_content(c, line_num):
            nonlocal tree
            try:
                tree = ast.parse(c)
                metrics.add_line_count(line_num)
                metrics.add_file_count()
            except SyntaxError as e:
                print(f"Syntax error in file {e.filename} at (line - {e.lineno}, column - {e.offset}): {e.text}")

        process_file_content(file_info.path, handle_file_content)

        if tree is None:
            return
        file_info.preprocess_file(tree)
        self.handle_modules_outside_of_project(file_info)
        for dependency in file_info.preprocessed_file.import_table.get_dependencies():
            dependency_file_info = [x for x in self.files if x.path == dependency.location]
            if len(dependency_file_info) == 0:
                pass
            elif len(dependency_file_info) == 1:
                if dependency_file_info[0].status is ProcessStatus.WAITING:
                    self.parse_file(dependency_file_info[0])
                    current_file = file_info.get_file()
                    logger.debug('\nFILE: ' + file_info.get_file_name() + '\n=========================================')
                    db_logger. \
                        debug('\nFILE: ' + file_info.get_file_name() + '\n=========================================')
            else:
                assert False, 'Multiple file occurrence: ' + dependency.get_file()
        sc = SymbolCollector(ParserTree(tree), file_info.path, file_info.preprocessed_file,
                             self, self.persistence, self)
        # print(current_file)
        sc.collect_symbols()
        file_info.set_variable_collector(sc)
        assert(isinstance(sc.scope_manager.get_current_scope(), GlobalScope))
        self.scope_managers.append(sc.scope_manager)
        # file_info.symbol_collector.scope_manager.persist_current_scope()

        self.persistence.persist_file(file_info.create_dto())

    def handle_modules_outside_of_project(self, file_info: FileInfo) -> None:
        for m in file_info.preprocessed_file.import_table.import_paths:
            # TODO: after python 3.9 use PurePath.is_relative_to instead of 'in parents'
            if not any(PurePath(mm) in m.parents for mm in self.directories) and \
                    m not in self.other_modules:
                self.other_modules.add(m)
                new_file_info = FileInfo(m)
                self.other_module_files.append(new_file_info)
                self.parse_file(new_file_info)

    def get_file_by_location(self, location: PurePath) -> Optional[FileInfo]:
        for file in self.files:
            if file.path == location:
                return file
        return None

    def persist_builtins(self):
        for bit in get_all_built_in_types():
            self.persistence.persist_preprocessed_class(bit.create_dto())

        for bif in get_all_built_in_functions():
            self.persistence.persist_function(bif.create_dto())

    def persist_global_scopes(self):
        for scope_manager in self.scope_managers:
            scope_manager.persist_current_scope()

    def get_function_symbol_collector(self, symbol_collector: SymbolCollector,
                                      func_def: Union[ast.FunctionDef, ast.AsyncFunctionDef],
                                      arguments: List[Tuple[DeclarationType, Optional[str]]]) \
            -> IFunctionSymbolCollector:
        return FunctionSymbolCollector(symbol_collector, func_def, arguments)
