from pathlib import PurePath
from typing import Dict, List

from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.persistence.base_dto import create_file_position_dto
from cc_python_parser.persistence.file_position_dto import FilePositionDTO


class ImportDataDTO:
    def __init__(self, qualified_name: str, imported: PurePath, pos: FilePosition):
        self.qualified_name: str = qualified_name
        self.imported: str = str(imported)
        self.position: FilePositionDTO = create_file_position_dto(pos)

    def __hash__(self):
        return hash(self.imported) ^ hash(self.position)

    def __eq__(self, other):
        return self.imported == other.imported and self.position == other.position

    def __ne__(self, other):
        return not self.__eq__(other)


class ImportDTO:
    def __init__(self, importer: PurePath):
        self.importer: str = str(importer)
        self.imported_modules: List[ImportDataDTO] = []
        self.imported_symbols: Dict[ImportDataDTO, List[str]] = {}

    def add_module_import(self, qualified_name: str, imported: PurePath, pos: FilePosition):
        self.imported_modules.append(ImportDataDTO(qualified_name, imported, pos))

    def add_symbol_import(self, qualified_name: str, imported: PurePath, pos: FilePosition, symbol_qualified_name: str):
        imported_data_dto = ImportDataDTO(qualified_name, imported, pos)
        if imported_data_dto not in self.imported_symbols:
            self.imported_symbols[imported_data_dto] = []
        self.imported_symbols[imported_data_dto].append(symbol_qualified_name)
