from typing import List

from my_ast.common.file_position import FilePosition
from my_ast.base_data import TypeDeclaration, ImportedDeclaration, DocumentedType
import my_ast.function_data as fd
import my_ast.variable_data as vd
from my_ast.persistence.base_dto import UsageDTO

from my_ast.persistence.class_dto import ClassDeclarationDTO, ClassMembersDTO


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
        members = ClassMembersDTO()
        for m in self.methods:
            members.methods.append(m.qualified_name)
        for sm in self.static_methods:
            members.static_methods.append(sm.qualified_name)
        for a in self.attributes:
            members.attributes.append(a.qualified_name)
        for sa in self.static_attributes:
            members.static_attributes.append(sa.qualified_name)
        for c in self.classes:
            members.classes.append(c.qualified_name)
        return ClassDeclarationDTO(self.name, self.qualified_name, self.file_position,
                                   types, usages, base_classes, members, self.documentation)

    def get_type_repr(self) -> str:
        return self.name


class ImportedClassDeclaration(ClassDeclaration, ImportedDeclaration[ClassDeclaration]):
    def __init__(self, name: str, pos: FilePosition, class_declaration: ClassDeclaration):
        ClassDeclaration.__init__(self, name, "", pos, class_declaration.base_classes, "")
        ImportedDeclaration.__init__(self, class_declaration)
        self.type = {class_declaration}
