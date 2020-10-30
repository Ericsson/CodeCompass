from typing import List, Union, Set, Optional

import my_ast.base_data as data
from my_ast.built_in_types import BuiltIn
from my_ast.class_data import ClassDeclaration
from my_ast.common.file_position import FilePosition
from my_ast.function_data import FunctionDeclaration, FunctionParameter


class ClassInitDeclaration(FunctionDeclaration):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition, params: List[FunctionParameter],
                 class_declaration: ClassDeclaration,
                 func_type: Optional[Set[Union[data.Declaration, BuiltIn]]] = None):
        super().__init__(name, qualified_name, pos, params, "",
                         func_type if func_type is not None else {class_declaration})
        self.class_declaration = class_declaration
