from typing import List, Optional

from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.base_data import TypeDeclaration, ImportedDeclaration, DocumentedType
import cc_python_parser.function_data as fd
import cc_python_parser.variable_data as vd
from cc_python_parser.persistence.base_dto import UsageDTO

from cc_python_parser.persistence.class_dto import ClassDeclarationDTO


# TODO: @classmethod
class ClassDeclaration(TypeDeclaration, DocumentedType):
    def __init__(self, name: str, qualified_name: str, position: FilePosition, base_classes: List['ClassDeclaration'],
                 documentation: str):
        TypeDeclaration.__init__(self, name, qualified_name, position)
        DocumentedType.__init__(self, documentation)
        self.base_classes: List[ClassDeclaration] = base_classes
        self.methods: List[fd.FunctionDeclaration] = []
        self.static_methods: List[fd.StaticFunctionDeclaration] = []
        self.attributes: List[vd.VariableDeclaration] = []
        self.static_attributes: List[vd.StaticVariableDeclaration] = []
        self.classes: List[ClassDeclaration] = []

    def create_dto(self) -> ClassDeclarationDTO:
        usages = []
        for usage in self.usages:
            usages.append(UsageDTO(usage.file_position))
        types = set()
        base_classes = set()
        for base_class in self.base_classes:
            base_classes.add(base_class.qualified_name)
        members = ClassDeclarationDTO.ClassMembersDTO()
        for m in self.methods:
            members.methods.append(m.create_dto())
        for sm in self.static_methods:
            members.static_methods.append(sm.create_dto())
        for a in self.attributes:
            members.attributes.append(a.create_dto())
        for sa in self.static_attributes:
            members.static_attributes.append(sa.create_dto())
        for c in self.classes:
            members.classes.append(c.create_dto())
        return ClassDeclarationDTO(self.name, self.qualified_name, self.file_position,
                                   types, usages, base_classes, members, self.documentation)

    def get_init_method(self) -> Optional[fd.FunctionDeclaration]:
        for m in reversed(self.methods):
            if m.name == '__init__':
                return m
        return None     # has default init

    def get_type_repr(self) -> str:
        return self.name


class ImportedClassDeclaration(ClassDeclaration, ImportedDeclaration[ClassDeclaration]):
    def __init__(self, qualified_name: str, name: str, pos: FilePosition, class_declaration: ClassDeclaration, module):
        ClassDeclaration.__init__(self, name, "", pos, class_declaration.base_classes, "")
        ImportedDeclaration.__init__(self, qualified_name, class_declaration, module, pos)
        self.type = {class_declaration}
