from typing import List, Optional, Set, Union

from my_ast.common.file_position import FilePosition
import my_ast.base_data as data
from my_ast.persistence.base_dto import UsageDTO
from my_ast.persistence.function_dto import FunctionDeclarationDTO
from my_ast.type_data import DeclarationType


class FunctionParameter:
    def __init__(self, name: str, func_type: str = None):
        self.name: str = name
        self.type: Optional[str] = func_type


class FunctionDeclaration(data.Declaration, data.DocumentedType):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition, params: List[FunctionParameter],
                 documentation: str, func_type: Optional[Set[Union[DeclarationType]]] = None):
        data.Declaration.__init__(self, name, qualified_name, pos, func_type)
        data.DocumentedType.__init__(self, documentation)
        self.parameters: List[FunctionParameter] = params

    def create_dto(self) -> FunctionDeclarationDTO:
        usages = []
        for usage in self.usages:
            usages.append(UsageDTO(usage.file_position))
        types = set()
        for t in self.type:
            types.add(t.qualified_name)
        return FunctionDeclarationDTO(self.name, self.qualified_name, self.file_position,
                                      types, usages, self.documentation)


class StaticFunctionDeclaration(FunctionDeclaration):
    pass


class ImportedFunctionDeclaration(FunctionDeclaration, data.ImportedDeclaration[FunctionDeclaration]):
    def __init__(self, name: str, pos: FilePosition, func_declaration: FunctionDeclaration):
        FunctionDeclaration.__init__(self, name, "", pos, func_declaration.parameters, "", func_declaration.type)
        data.ImportedDeclaration.__init__(self, func_declaration)
