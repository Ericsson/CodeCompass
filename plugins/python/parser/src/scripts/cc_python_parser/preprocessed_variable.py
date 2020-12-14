from typing import List

from cc_python_parser.base_data import Declaration
from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.preprocessed_data import PreprocessedDeclaration


class PreprocessedVariable(PreprocessedDeclaration):
    def __init__(self, name: str, file_position: FilePosition):
        super().__init__(name, file_position)
        self.type_usages: List[Declaration] = []

    def __eq__(self, other):
        return isinstance(other, PreprocessedVariable) and self.name == other.name

    def __ne__(self, other):
        return not self.__eq__(other)

    def __hash__(self):
        return hash(self.name) ^ hash(self.name)
