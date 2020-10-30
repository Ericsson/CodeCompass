from pathlib import PurePath
from typing import Set, Dict


class ImportDTO:
    def __init__(self, importer: PurePath):
        self.importer: str = str(importer)
        self.imported_modules: Set[str] = set()
        self.imported_symbols: Dict[str, Set[str]] = {}

    def add_module_import(self, imported: PurePath):
        self.imported_modules.add(str(imported))

    def add_symbol_import(self, imported: PurePath, symbol_qualified_name: str):
        if str(imported) not in self.imported_symbols:
            self.imported_symbols[str(imported)] = set()
        self.imported_symbols[str(imported)].add(symbol_qualified_name)
