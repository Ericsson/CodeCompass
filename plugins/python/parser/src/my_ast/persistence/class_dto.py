from typing import Set, List

from my_ast.common.file_position import FilePosition
from my_ast.persistence.base_dto import UsageDTO, DeclarationDTO
from my_ast.persistence.documented_dto import DocumentedDTO


class ClassMembersDTO:
    def __init__(self):
        self.methods: List[str] = []
        self.static_methods: List[str] = []
        self.attributes: List[str] = []
        self.static_attributes: List[str] = []
        self.classes: List[str] = []


class ClassDeclarationDTO(DeclarationDTO, DocumentedDTO):
    def __init__(self, name: str, qualified_name: str, file_position: FilePosition, types: Set[str],
                 usages: List[UsageDTO], base_classes: Set[str], members: ClassMembersDTO, documentation: str):
        DeclarationDTO.__init__(self, name, qualified_name, file_position, types, usages)
        DocumentedDTO.__init__(self, documentation)
        self.base_classes: Set[str] = base_classes
        self.members: ClassMembersDTO = members
