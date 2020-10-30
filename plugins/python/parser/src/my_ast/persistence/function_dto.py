from typing import Set, List

from my_ast.common.file_position import FilePosition
from my_ast.persistence.base_dto import DeclarationDTO, UsageDTO
from my_ast.persistence.documented_dto import DocumentedDTO


class FunctionDeclarationDTO(DeclarationDTO, DocumentedDTO):
    def __init__(self, name: str, qualified_name: str, file_position: FilePosition, types: Set[str],
                 usages: List[UsageDTO], documentation: str):
        DeclarationDTO.__init__(self, name, qualified_name, file_position, types, usages)
        DocumentedDTO.__init__(self, documentation)
