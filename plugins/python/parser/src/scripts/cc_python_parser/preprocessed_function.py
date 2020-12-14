import ast
from typing import List, Union

from cc_python_parser.base_data import Declaration
from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.preprocessed_data import PreprocessedDeclaration


class PreprocessedFunction(PreprocessedDeclaration):
    def __init__(self, name: str, file_position: FilePosition, node: Union[ast.FunctionDef, ast.AsyncFunctionDef]):
        super().__init__(name, file_position)
        self.type_usages: List[Declaration] = []
        self.node: Union[ast.FunctionDef, ast.AsyncFunctionDef] = node

    def __eq__(self, other):
        return isinstance(other, PreprocessedFunction) and self.name == other.name

    def __ne__(self, other):
        return not self.__eq__(other)

    def __hash__(self):
        return hash(self.name) ^ hash(self.name)
