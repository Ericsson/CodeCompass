import ast
from typing import Dict, Tuple, Optional, Set, Union, List

from cc_python_parser.common.hashable_list import OrderedHashableList
from cc_python_parser.function_data import FunctionDeclaration
from cc_python_parser.type_data import DeclarationType


# TODO: redefined functions?
class PlaceholderFunctionDeclarationCache:
    class FunctionDeclarationData:
        def __init__(self, func: Union[ast.FunctionDef, ast.AsyncFunctionDef], is_method: bool):
            self.function = func
            self.is_method = is_method

    def __init__(self):
        self.cache: \
            Dict[FunctionDeclaration,
                 Tuple[PlaceholderFunctionDeclarationCache.FunctionDeclarationData,
                       Dict[OrderedHashableList[DeclarationType], Set[DeclarationType]]]] = {}

    def add_function_declaration(self, declaration: FunctionDeclaration, function_def: FunctionDeclarationData) -> None:
        self.cache[declaration] = (function_def, {})

    def get_function_def(self, declaration: FunctionDeclaration) -> Optional[FunctionDeclarationData]:
        if declaration in self.cache:
            return self.cache[declaration][0]
        return None

    def get_function_return_type(self, declaration: FunctionDeclaration,
                                 param_types: OrderedHashableList[DeclarationType]) -> Optional[Set[DeclarationType]]:
        if declaration in self.cache and param_types in self.cache[declaration][1]:
            return self.cache[declaration][1][param_types]
        return None

    def get_functions_return_type(self, declarations: Set[FunctionDeclaration],
                                  param_types: OrderedHashableList[DeclarationType]) \
            -> Set[DeclarationType]:
        types = set()
        for declaration in declarations:
            t = self.get_function_return_type(declaration, param_types)
            if t is not None:
                types.update(t)
        return types

    def add_function_return_type(self, declaration: FunctionDeclaration,
                                 param_types: OrderedHashableList[DeclarationType], return_type: Set[DeclarationType]) \
            -> None:
        if declaration in self.cache and param_types not in self.cache[declaration][1]:
            self.cache[declaration][1][param_types] = return_type
        elif declaration not in self.cache:
            assert False, "Key not found"
