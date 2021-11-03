from abc import ABC, abstractmethod
from typing import Dict

from cc_python_parser.base_data import ImportedDeclaration
from cc_python_parser.import_finder import ImportFinder
from cc_python_parser.member_access_collector import MemberAccessCollector
from cc_python_parser.preprocessed_file import PreprocessedFile
from cc_python_parser.scope_manager import ScopeManager


class ISymbolCollector(ABC):
    @abstractmethod
    def collect_symbols(self):
        pass


class SymbolCollectorBase(ISymbolCollector, ABC):
    def __init__(self, preprocessed_file: PreprocessedFile, import_finder: ImportFinder):
        self.preprocessed_file: PreprocessedFile = preprocessed_file
        self.import_finder: ImportFinder = import_finder
        self.member_access_collector_type = MemberAccessCollector
        self.imported_declaration_scope_map: Dict[ImportedDeclaration, ScopeManager] = {}


class IFunctionSymbolCollector(ISymbolCollector, ABC):
    def __init__(self):
        self.function = None
