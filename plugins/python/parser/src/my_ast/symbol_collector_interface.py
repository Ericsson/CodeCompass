from abc import ABC, abstractmethod
from typing import Dict

from my_ast.base_data import ImportedDeclaration
from my_ast.import_finder import ImportFinder
from my_ast.member_access_collector import MemberAccessCollector
from my_ast.preprocessed_file import PreprocessedFile
from my_ast.scope_manager import ScopeManager


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
