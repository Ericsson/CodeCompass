from typing import List, Optional, Set, Union

from cc_python_parser.built_in_types import AnyType, Method, Function, Type
from cc_python_parser.common.file_position import FilePosition
import cc_python_parser.base_data as data
from cc_python_parser.persistence.base_dto import UsageDTO
from cc_python_parser.persistence.function_dto import FunctionDeclarationDTO
from cc_python_parser.type_data import DeclarationType, PlaceholderType


class FunctionDeclaration(data.Declaration, data.DocumentedType):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition, params: List[data.Declaration],
                 documentation: str, func_type: Optional[Set[Union[DeclarationType]]] = None):
        data.Declaration.__init__(self, name, qualified_name, pos, func_type)
        data.DocumentedType.__init__(self, documentation)
        self.parameters: List[data.Declaration] = params
        self.local_variables: List[data.Declaration] = []

    def create_dto(self) -> FunctionDeclarationDTO:
        usages = []
        for usage in self.usages:
            usages.append(UsageDTO(usage.file_position))
        types = set()
        for t in self.type:
            if isinstance(t, PlaceholderType):
                types.add(AnyType().qualified_name)
            # elif isinstance(t, MethodVariableDeclaration):
            #     types.add(Method().qualified_name)
            # elif isinstance(t, FunctionVariableDeclaration):
            #     types.add(Function().qualified_name)
            # elif isinstance(t, TypeVariableDeclaration):
            #     types.add(Type().qualified_name)
            else:
                types.add(t.qualified_name)
        params = []
        for param in self.parameters:
            params.append(param.qualified_name)
        local_vars = []
        for local in self.local_variables:
            local_vars.append(local.qualified_name)
        return FunctionDeclarationDTO(self.name, self.qualified_name, self.file_position,
                                      types, usages, self.documentation, params, local_vars)


class StaticFunctionDeclaration(FunctionDeclaration):
    pass


class ImportedFunctionDeclaration(FunctionDeclaration, data.ImportedDeclaration[FunctionDeclaration]):
    def __init__(self, qualified_name: str, name: str, pos: FilePosition, func_declaration: FunctionDeclaration, module):
        FunctionDeclaration.__init__(self, name, "", pos, func_declaration.parameters, "", func_declaration.type)
        data.ImportedDeclaration.__init__(self, qualified_name, func_declaration, module, pos)
