from typing import Set, List

from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.persistence.base_dto import DeclarationDTO, UsageDTO
from cc_python_parser.persistence.documented_dto import DocumentedDTO


class FunctionDeclarationDTO(DeclarationDTO, DocumentedDTO):
    def __init__(self, name: str, qualified_name: str, file_position: FilePosition, types: Set[str],
                 usages: List[UsageDTO], documentation: str, params: List[str], local_vars: List[str]):
        DeclarationDTO.__init__(self, name, qualified_name, file_position, types, usages)
        DocumentedDTO.__init__(self, documentation)
        self.parameters: List[str] = params
        self.locals: List[str] = local_vars
