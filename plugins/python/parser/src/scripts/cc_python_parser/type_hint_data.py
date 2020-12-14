from typing import Union

from cc_python_parser.built_in_types import BuiltIn
from cc_python_parser.class_data import ClassDeclaration
from cc_python_parser.type_data import DeclarationType


class TypeHintType(DeclarationType):
    def __init__(self, hinted_type: Union[ClassDeclaration, BuiltIn]):
        super().__init__(hinted_type.name, hinted_type.qualified_name)
        self.hinted_type = hinted_type

    def get_type_repr(self) -> str:
        return 'Hinted type: ' + self.hinted_type.get_type_repr()

    def __hash__(self):
        return hash(self.hinted_type)

    def __eq__(self, other):
        return isinstance(other, TypeHintType) and self.hinted_type == other.hinted_type

    def __ne__(self, other):
        return not self.__eq__(other)
