from typing import List

from my_ast.base_data import Declaration
from my_ast.preprocessed_data import PreprocessedDeclaration


class PreprocessedVariable(PreprocessedDeclaration):
    def __init__(self, name):
        super().__init__(name)
        self.type_usages: List[Declaration] = []

    def __eq__(self, other):
        return isinstance(other, PreprocessedVariable) and self.name == other.name

    def __ne__(self, other):
        return not self.__eq__(other)

    def __hash__(self):
        return hash(self.name) ^ hash(self.name)
