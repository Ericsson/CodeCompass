from typing import Set, List

from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.persistence.base_dto import UsageDTO, DeclarationDTO
from cc_python_parser.persistence.documented_dto import DocumentedDTO
from cc_python_parser.persistence.variable_dto import VariableDeclarationDTO
from cc_python_parser.persistence.function_dto import FunctionDeclarationDTO


class ClassDeclarationDTO(DeclarationDTO, DocumentedDTO):
    class ClassMembersDTO:
        def __init__(self):
            self.methods: List[FunctionDeclarationDTO] = []
            self.static_methods: List[FunctionDeclarationDTO] = []
            self.attributes: List[VariableDeclarationDTO] = []
            self.static_attributes: List[VariableDeclarationDTO] = []
            self.classes: List[ClassDeclarationDTO] = []

    def __init__(self, name: str, qualified_name: str, file_position: FilePosition,
                 types: Set[str], usages: List[UsageDTO], base_classes: Set[str],
                 members: 'ClassDeclarationDTO.ClassMembersDTO', documentation: str):
        DeclarationDTO.__init__(self, name, qualified_name, file_position, types, usages)
        DocumentedDTO.__init__(self, documentation)
        self.base_classes: List[str] = list(base_classes)
        self.members: ClassDeclarationDTO.ClassMembersDTO = members
