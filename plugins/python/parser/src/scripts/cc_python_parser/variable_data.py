from pathlib import PurePath
from typing import Optional, Set

from cc_python_parser.built_in_types import Module, AnyType, Method, Function, Type
from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.base_data import Declaration, TypeDeclaration, ImportedDeclaration, ReferenceDeclaration
from cc_python_parser.function_data import FunctionDeclaration
from cc_python_parser.persistence.base_dto import UsageDTO
from cc_python_parser.persistence.variable_dto import VariableDeclarationDTO
from cc_python_parser.type_data import PlaceholderType


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
            if isinstance(t, PlaceholderType):
                types.add(AnyType().qualified_name)
            elif isinstance(t, MethodVariableDeclaration):
                types.add(Method().qualified_name)
            elif isinstance(t, FunctionVariableDeclaration):
                types.add(Function().qualified_name)
            elif isinstance(t, TypeVariableDeclaration):
                types.add(Type().qualified_name)
            elif isinstance(t, ImportedDeclaration):
                types.add(t.imported_declaration.qualified_name)
            elif isinstance(t, ReferenceDeclaration):
                types.add(t.reference.qualified_name)
            else:
                types.add(t.qualified_name)
        return VariableDeclarationDTO(self.name, self.qualified_name, self.file_position, types, usages)


class StaticVariableDeclaration(VariableDeclaration):
    pass


class ReferenceVariableDeclaration(VariableDeclaration, ReferenceDeclaration):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition, reference: Declaration):
        VariableDeclaration.__init__(self, name, qualified_name, pos, reference.type)
        ReferenceDeclaration.__init__(self, reference)


class NonlocalVariableDeclaration(ReferenceVariableDeclaration):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition, reference: VariableDeclaration):
        super().__init__(name, qualified_name, pos, reference)


class GlobalVariableDeclaration(ReferenceVariableDeclaration):
    def __init__(self, name: str, qualified_name: str, pos: FilePosition, reference: VariableDeclaration):
        super().__init__(name, qualified_name, pos, reference)


# TODO: short/long name? (in case of import)
class ModuleVariableDeclaration(VariableDeclaration):
    # module: Union[GlobalScope, PreprocessedFile] - circular import
    def __init__(self, qualified_name: str, name: str, location: PurePath, pos: FilePosition, module):
        super().__init__(name, qualified_name, pos, {Module()})
        self.imported_module = module
        self.imported_module_location: PurePath = location


class ImportedVariableDeclaration(VariableDeclaration, ImportedDeclaration[VariableDeclaration]):
    def __init__(self, qualified_name: str, name: str, pos: FilePosition, var_declaration: VariableDeclaration, module):
        VariableDeclaration.__init__(self, name, "", pos, var_declaration.type)
        ImportedDeclaration.__init__(self, qualified_name, var_declaration, module, pos)


class TypeVariableDeclaration(ReferenceVariableDeclaration):
    def __init__(self, type_ref: TypeDeclaration):
        super().__init__(type_ref.name, type_ref.qualified_name, type_ref.file_position, type_ref)

    def get_type_repr(self) -> str:
        return '[TypeVariable(' + self.reference.name + ')]'


class FunctionVariableDeclaration(ReferenceVariableDeclaration):
    def __init__(self, func_ref: FunctionDeclaration):
        super().__init__(func_ref.name, func_ref.qualified_name, func_ref.file_position, func_ref)

    def get_type_repr(self) -> str:
        return '[FunctionVariable(' + self.reference.name + ')]'


class MethodVariableDeclaration(FunctionVariableDeclaration):
    # self_var: ClassDeclaration
    def __init__(self, func_ref: FunctionDeclaration, self_var):
        super().__init__(func_ref)
        self.self = self_var

    def get_type_repr(self) -> str:
        return '[MethodVariable(' + self.reference.name + ')]'
