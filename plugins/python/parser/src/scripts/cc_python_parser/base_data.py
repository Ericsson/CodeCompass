from typing import List, TypeVar, Generic, Set, Optional

from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.type_data import DeclarationType


class Usage:
    def __init__(self, name: str, position: FilePosition):
        self.name = name
        self.file_position = position

    def __str__(self):
        return self.name + ' (' + str(self.file_position) + ')'

    def __repr__(self):
        return self.__str__()


class Declaration(DeclarationType):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition,
                 declaration_type: Optional[Set[DeclarationType]] = None):
        DeclarationType.__init__(self, name, qualified_name)
        assert declaration_type is None or not any(isinstance(x, type) for x in declaration_type)
        self.file_position: FilePosition = pos
        if declaration_type is None:
            self.type: Set[DeclarationType] = set()
        else:
            self.type: Set[DeclarationType] = declaration_type
        self.usages: List[Usage] = []

    def is_same_declaration(self, other: 'Declaration') -> bool:
        return self.__eq__(other) and self.file_position == other.file_position

    def is_same_usage(self, other: Usage):
        return self.name == other.name and self.file_position == other.file_position

    def __eq__(self, other):
        return isinstance(other, type(self)) and self.name == other.name

    def __ne__(self, other):
        return not self.__eq__(other)

    def __hash__(self):
        return hash(self.qualified_name) ^ hash(self.file_position)

    def get_type_repr(self) -> str:
        return '[' + ','.join({x.get_type_repr() if x is not None else 'None' for x in self.type}) + ']'


class TypeDeclaration(Declaration):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition):
        super().__init__(name, qualified_name, pos)
        self.type.add(self)


T = TypeVar('T', bound=Declaration)


class ImportedDeclaration(Generic[T]):
    def __init__(self, qualified_name: str, declaration: T, module, pos: FilePosition):
        self.qualified_name = qualified_name
        self.module = module
        self.position = pos


class ReferenceDeclaration:
    def __init__(self, reference: Declaration):
        self.reference: Declaration = reference


class DocumentedType:
    def __init__(self, documentation: str):
        self.documentation: str = documentation
