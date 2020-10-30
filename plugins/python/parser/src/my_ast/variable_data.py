from pathlib import PurePath
from typing import Optional, Set, Union

from my_ast.built_in_types import Module, BuiltIn
from my_ast.common.file_position import FilePosition
from my_ast.base_data import Declaration, TypeDeclaration, ImportedDeclaration
from my_ast.function_data import FunctionDeclaration
from my_ast.persistence.base_dto import UsageDTO
from my_ast.persistence.variable_dto import VariableDeclarationDTO


class VariableDeclaration(Declaration):
    # var_type: Optional[Set[Union[cd.ClassDeclaration, BuiltIn]]] - circular import
    def __init__(self, name: str, qualified_name: str, pos: FilePosition, declaration_type: Optional[Set] = None):
        super().__init__(name, qualified_name, pos, declaration_type)
        # self.type: VariableDeclaration and FunctionDeclaration type can change
        # (eg. new assignment with different type or redefinition)

    def create_dto(self) -> VariableDeclarationDTO:
        usages = []
        for usage in self.usages:
            usages.append(UsageDTO(usage.file_position))
        types = set()
        for t in self.type:
            types.add(t.qualified_name)
        return VariableDeclarationDTO(self.name, self.qualified_name, self.file_position, types, usages)


class StaticVariableDeclaration(VariableDeclaration):
    pass


class ReferenceVariableDeclaration(VariableDeclaration):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition, reference: Declaration):
        super().__init__(name, qualified_name, pos, reference.type)
        self.reference: Declaration = reference


class NonlocalVariableDeclaration(ReferenceVariableDeclaration):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition, reference: VariableDeclaration):
        super().__init__(name, qualified_name, pos, reference)


class GlobalVariableDeclaration(ReferenceVariableDeclaration):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition, reference: VariableDeclaration):
        super().__init__(name, qualified_name, pos, reference)


# TODO: short/long name? (in case of import)
class ModuleVariableDeclaration(VariableDeclaration):
    # module: Union[GlobalScope, PreprocessedFile] - circular import
    def __init__(self, name: str, location: PurePath, pos: FilePosition, module):
        super().__init__(name, "", pos, {Module()})     # TODO: need qualified name?
        self.imported_module = module
        self.imported_module_location: PurePath = location


class ImportedVariableDeclaration(VariableDeclaration, ImportedDeclaration[VariableDeclaration]):
    def __init__(self, name: str, pos: FilePosition, var_declaration: VariableDeclaration):
        VariableDeclaration.__init__(self, name, "", pos, var_declaration.type)
        ImportedDeclaration.__init__(self, var_declaration)


class TypeVariable(ReferenceVariableDeclaration):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition, type_ref: TypeDeclaration):
        super().__init__(name, qualified_name, pos, type_ref)

    def get_type_repr(self) -> str:
        return '[TypeVariable(' + self.reference.name + ')]'


class FunctionVariable(ReferenceVariableDeclaration):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition, func_ref: FunctionDeclaration):
        super().__init__(name, qualified_name, pos, func_ref)

    def get_type_repr(self) -> str:
        return '[FunctionVariable(' + self.reference.name + ')]'
