import ast
from pathlib import PurePath, Path
from typing import List, Type, Iterator, Callable, Optional, Union

from cc_python_parser.base_data import Declaration, ImportedDeclaration
from cc_python_parser.class_data import ClassDeclaration
from cc_python_parser.class_init_declaration import ClassInitDeclaration
from cc_python_parser.function_data import FunctionDeclaration
from cc_python_parser.import_finder import ImportFinder
from cc_python_parser.import_preprocessor import ImportTable, Import
from cc_python_parser.member_access_collector import MemberAccessCollectorIterator, MemberAccessCollector
from cc_python_parser.persistence.import_dto import ImportDTO
from cc_python_parser.persistence.persistence import ModelPersistence
from cc_python_parser.placeholder_function_declaration_cache import PlaceholderFunctionDeclarationCache
from cc_python_parser.scope import Scope, ClassScope, GlobalScope, FunctionScope, LifetimeScope, PartialLifetimeScope, \
    ConditionalScope, LoopScope, ImportScope
from cc_python_parser.type_data import PlaceholderType
from cc_python_parser.variable_data import VariableDeclaration, ModuleVariableDeclaration, FunctionVariableDeclaration, \
    TypeVariableDeclaration
from cc_python_parser.common.position import Range
from cc_python_parser.common.file_position import FilePosition


class ScopeManager:
    def __init__(self, current_file: PurePath, import_finder: ImportFinder, persistence: ModelPersistence,
                 scopes: Optional[List[Scope]] = None):
        self.current_file = current_file
        self.import_finder = import_finder
        if scopes is None:
            self.scopes: List[Scope] = [GlobalScope(self.get_qualified_scope_name_from_current_scope([]),
                                                    FilePosition(current_file, Range.get_empty_range()))]
        else:
            self.scopes = scopes
        self.persistence: ModelPersistence = persistence
        self.placeholder_function_declaration_cache = PlaceholderFunctionDeclarationCache()
        self.persist_symbols: int = 0

    def lock_persisting(self):
        self.persist_symbols += 1

    def unlock_persisting(self):
        self.persist_symbols -= 1

    def is_current_scope_instance(self, t: Type) -> bool:
        return isinstance(self.scopes[-1], t)

    # TODO: local functions
    def is_current_scope_method(self) -> bool:
        for i in range(len(self.scopes) - 1, -1, -1):
            if isinstance(self.scopes[i], LifetimeScope) and not isinstance(self.scopes[i], PartialLifetimeScope):
                return isinstance(self.scopes[i], FunctionScope) and isinstance(self.scopes[i - 1], ClassScope)
        return False

    def is_current_scope_init(self) -> bool:
        return self.is_current_scope_method() and self.get_current_lifetime_scope().name == '__init__'

    def get_current_lifetime_scope(self) -> LifetimeScope:
        for scope in self.reverse():
            if isinstance(scope, LifetimeScope) and not isinstance(scope, PartialLifetimeScope):
                return scope
        assert False  # there should be always a GlobalScope, which is LifetimeScope

    def reverse(self) -> Iterator[Scope]:
        for scope in reversed(self.scopes):
            yield scope

    def iterate(self) -> Iterator[Scope]:
        for scope in self.scopes:
            yield scope

    def with_scope(self, scope: Scope, action: Callable[[], None]) -> None:
        self.scopes.append(scope)
        action()
        self.persist_current_scope()
        del self.scopes[-1]

    def get_size(self) -> int:
        return len(self.scopes)

    def get_current_scope(self) -> Scope:
        return self.scopes[-1]

    def get_global_scope(self) -> GlobalScope:
        assert len(self.scopes) > 0
        global_scope = self.scopes[0]
        if not isinstance(global_scope, GlobalScope):
            assert False
        return global_scope

    def append_variable_to_current_scope(self, new_variable: VariableDeclaration) -> None:
        self.get_current_lifetime_scope().append_variable(new_variable)

    def append_exception_to_current_scope(self, new_variable: VariableDeclaration) -> None:
        self.get_current_partial_lifetime_scope().append_variable(new_variable)

    def append_function_to_current_scope(self, new_function: FunctionDeclaration) -> None:
        self.get_current_lifetime_scope().append_function(new_function)

    def append_class_to_current_scope(self, new_class: ClassDeclaration) -> None:
        self.get_current_lifetime_scope().append_class(new_class)

    def append_variable_to_current_class_scope(self, new_variable: VariableDeclaration) -> None:
        self.get_current_class_scope().append_variable(new_variable)

    def append_function_to_current_class_scope(self, new_function: FunctionDeclaration) -> None:
        self.get_current_class_scope().append_function(new_function)

    def append_class_to_current_class_scope(self, new_class: ClassDeclaration) -> None:
        self.get_current_class_scope().append_class(new_class)

    def append_module_variable_to_current_scope(self, module_variable: ModuleVariableDeclaration) -> None:
        def is_same_declaration(var: VariableDeclaration) -> bool:
            return isinstance(var, ModuleVariableDeclaration) and \
                   var.imported_module_location == module_variable.imported_module_location

        scope = self.get_current_lifetime_scope()
        if len([v for v in scope.variable_declarations if is_same_declaration(v)]) == 0:
            scope.append_variable(module_variable)

    def get_current_partial_lifetime_scope(self) -> Optional[LifetimeScope]:
        for scope in self.reverse():
            if isinstance(scope, PartialLifetimeScope):
                return scope
        return None

    def get_current_class_scope(self) -> Optional[ClassScope]:
        for scope in self.reverse():
            if isinstance(scope, ClassScope):
                return scope
        return None

    def get_current_scope_name(self) -> str:
        return self.scopes[-1].name

    def is_inside_conditional_scope(self) -> bool:
        for scope in self.reverse():
            if isinstance(scope, (ConditionalScope, LoopScope)):
                return True
            elif isinstance(scope, LifetimeScope) and not isinstance(scope, PartialLifetimeScope):
                return False
        assert False  # GlobalScope is LifetimeScope

    def is_declaration_in_current_scope(self, name: str) -> bool:
        return self.is_declaration_in_scope(name, self.scopes[-1])

    def is_declaration_in_current_lifetime_scope(self, name: str) -> bool:
        return self.is_declaration_in_scope(name, self.get_current_lifetime_scope())

    def is_declaration_in_current_class(self, name: str) -> bool:
        class_scope = self.get_current_class_scope()
        if class_scope is not None:
            return self.is_declaration_in_scope(name, class_scope)
        return False

    def get_declaration(self, name: str) -> Optional[Union[Declaration, PlaceholderType]]:
        for scope in self.reverse():
            if isinstance(scope, ClassScope) and scope is not self.get_current_lifetime_scope():
                continue  # members can be accessed via self, except from the current class scope
            declaration = self.get_declaration_from_scope(name, scope)
            if declaration is not None:
                from cc_python_parser.variable_data import TypeVariableDeclaration
                if isinstance(declaration, TypeVariableDeclaration):
                    pass  # print("NEED FIX: TypeVariable")
                return declaration
        current_class_scope = self.get_current_class_scope()
        if current_class_scope is not None:
            for init_placeholder in current_class_scope.init_placeholders:
                if name == init_placeholder.name:
                    return init_placeholder
        return PlaceholderType(name)

    def get_variable_declaration(self, name: str) -> Union[VariableDeclaration, PlaceholderType]:
        for scope in self.reverse():
            if isinstance(scope, ClassScope) and scope is not self.scopes[-1]:
                continue
            declaration = self.get_variable_declaration_from_scope(name, scope)
            if declaration is not None:
                return declaration
        return PlaceholderType(name)

    def get_function_declaration(self, name: str) -> Union[Declaration, PlaceholderType]:
        for scope in self.reverse():
            if isinstance(scope, ClassScope) and scope is not self.scopes[-1]:
                continue
            declaration = self.get_function_declaration_from_scope(name, scope)
            if declaration is not None:
                return declaration
        return PlaceholderType(name)

    def get_class_declaration(self, name: str) -> Union[ClassDeclaration, PlaceholderType]:
        for scope in self.reverse():
            if isinstance(scope, ClassScope) and scope is not self.scopes[-1]:
                continue
            declaration = self.get_class_declaration_from_scope(name, scope)
            if declaration is not None:
                return declaration
        return PlaceholderType(name)

    def get_declaration_from_member_access(self, iterator: MemberAccessCollectorIterator) -> Optional[Declaration]:
        iter(iterator)
        members = [next(iterator)]
        # TODO: MethodVariable?
        if isinstance(members[0], MemberAccessCollector.MethodData):
            declaration = self.get_function_declaration(members[0].name)
        elif isinstance(members[0], MemberAccessCollector.AttributeData):
            declaration = self.get_variable_declaration(members[0].name)
        else:
            declaration = self.get_declaration(members[0].name)
        if not isinstance(declaration, PlaceholderType):
            return declaration
        try:
            module_variable = self.get_imported_module(iterator)
            if module_variable is not None:
                declaration = module_variable
            else:
                declaration_from_other_module = self.get_declaration_from_other_module(iterator)
                if declaration_from_other_module is not None:
                    declaration = declaration_from_other_module
        except StopIteration:
            assert False
        return declaration

    def get_scope_manager_from_member_access(self, iterator: MemberAccessCollectorIterator) -> Optional['ScopeManager']:
        iter(iterator)
        members = [next(iterator)]
        if isinstance(members[0], MemberAccessCollector.MethodData):
            declaration = self.get_function_declaration(members[0].name)
        else:
            declaration = self.get_declaration(members[0].name)
        if not isinstance(declaration, PlaceholderType):
            if isinstance(declaration, ImportedDeclaration):
                return self.import_finder.get_scope_manager_by_location(declaration.module)
            else:
                return self
        try:
            module_import = self.get_import(iterator)
            if module_import is not None:
                return self.import_finder.get_scope_manager_by_location(module_import.location)
        except StopIteration:
            assert False
        return None

    # import a.b.c -> c.f() vs a.b.c.f()
    def get_declaration_from_other_module(self, iterator: MemberAccessCollectorIterator) -> Optional[Declaration]:
        for scope in self.reverse():
            if isinstance(scope, ImportScope):
                for module in reversed(scope.import_table.modules):
                    iter(iterator)
                    # TODO: is this alias check enough?
                    if module.module.alias is None and len(module.path) >= len(iterator.mac.call_list):
                        continue
                    is_same = True
                    current = next(iterator)
                    name = current.name
                    iter(iterator)
                    if module.module.alias is not None:
                        is_same = module.module.alias == name
                        next(iterator)
                    else:
                        for i in range(0, len(module.path)):
                            name = next(iterator).name
                            if module.path[i] != name:
                                is_same = False
                                break
                    if is_same:
                        name = next(iterator).name
                        if module.is_module_import():
                            other_scope_manager = self.import_finder.get_scope_manager_by_location(module.location)
                            if other_scope_manager is not None:
                                if isinstance(current, MemberAccessCollector.MemberData):
                                    return other_scope_manager.get_function_declaration(name)
                                else:
                                    return other_scope_manager.get_declaration(name)
                        for imp in module.imported:
                            if (imp.alias is not None and imp.alias == name) \
                                    or imp.name == name or imp.is_all_imported():
                                other_scope_manager = self.import_finder.get_scope_manager_by_location(module.location)
                                if other_scope_manager is not None:
                                    if isinstance(current, MemberAccessCollector.MemberData):
                                        return other_scope_manager.get_function_declaration(name)
                                    else:
                                        return other_scope_manager.get_declaration(name)
        return None

    def get_imported_module(self, iterator: MemberAccessCollectorIterator) -> Optional[ModuleVariableDeclaration]:
        imported_module = None
        for scope in self.reverse():
            if isinstance(scope, ImportScope):
                for module in reversed(scope.import_table.modules):
                    iter(iterator)
                    if module.module.alias is None and len(module.path) != len(iterator.mac.call_list):
                        continue
                    if module.module.alias is not None:
                        if module.module.alias == iterator.mac.call_list[-1].name:
                            imported_module = module
                            break
                        else:
                            continue
                    else:
                        for i in range(0, len(module.path)):
                            next(iterator)
                        module_path = [m.name for m in iterator.mac.call_list]
                        module_path.reverse()
                        if module.path == module_path:
                            imported_module = module
                            break
                if imported_module is not None and isinstance(scope, Scope):
                    for var in reversed(scope.variable_declarations):
                        if isinstance(var, ModuleVariableDeclaration) and \
                                var.imported_module_location == imported_module.location:
                            return var
        return None

    def get_import(self, iterator: MemberAccessCollectorIterator) -> Optional[Import]:
        for scope in self.reverse():
            if isinstance(scope, ImportScope):
                for module in reversed(scope.import_table.modules):
                    iter(iterator)
                    # TODO: is this alias check enough?
                    if module.module.alias is None and len(module.path) >= len(iterator.mac.call_list):
                        continue
                    current = next(iterator)
                    name = current.name
                    iter(iterator)
                    if module.module.alias is not None:
                        if module.module.alias == name:
                            return module
                    else:
                        for i in range(0, len(module.path)):
                            name = next(iterator).name
                            if module.path[i] != name:
                                return module
        return None

    def get_declaration_from_current_class(self, name: str) -> Union[Declaration, PlaceholderType]:
        declaration = self.get_declaration_from_scope(name, self.get_current_class_scope())
        if declaration is None:
            return PlaceholderType(name)
        return declaration

    def get_nonlocal_variable(self, name: str) -> Optional[VariableDeclaration]:
        assert len(self.scopes) > 2
        for scope in self.scopes[-2:0:-1]:
            var = scope.get_variable_by_name(name)
            if var is not None:
                return var
        return None

    def get_global_variable(self, name: str) -> Optional[VariableDeclaration]:
        return self.scopes[0].get_variable_by_name(name)

    @staticmethod
    def get_declaration_from_scope(name: str, scope: Scope) -> Optional[Declaration]:
        for declaration in reversed(scope.declarations):
            if declaration.name == name:
                return declaration
        return None

    @staticmethod
    def is_declaration_in_scope(name: str, scope: Scope) -> bool:
        for var in scope.declarations:
            if var.name == name:
                return True
        return False

    @staticmethod
    def get_variable_declaration_from_scope(name: str, scope: Scope) -> Optional[VariableDeclaration]:
        for declaration in reversed(scope.declarations):
            if declaration.name != name:
                continue
            elif isinstance(declaration, VariableDeclaration):
                return declaration
            elif isinstance(declaration, FunctionDeclaration) and not isinstance(declaration, ClassInitDeclaration):
                return FunctionVariableDeclaration(declaration)
            elif isinstance(declaration, ClassDeclaration):
                return TypeVariableDeclaration(declaration)
        return None

    @staticmethod
    def get_function_declaration_from_scope(name: str, scope: Scope) \
            -> Optional[Union[FunctionDeclaration, ClassDeclaration, FunctionVariableDeclaration]]:
        for declaration in reversed(scope.declarations):
            if declaration.name != name:
                continue
            elif isinstance(declaration, (FunctionDeclaration, ClassDeclaration)):
                return declaration
            elif isinstance(declaration, ClassDeclaration):
                return ClassInitDeclaration(declaration)
            else:
                function_variables = [t for t in declaration.type if isinstance(t, FunctionVariableDeclaration)]
                if len(function_variables) > 0:
                    return function_variables[0]
                else:
                    return declaration
        return None

    @staticmethod
    def get_class_declaration_from_scope(name: str, scope: Scope) -> Optional[ClassDeclaration]:
        for declaration in reversed(scope.class_declarations):
            if declaration.name == name:
                return declaration
        return None

    def get_current_class_declaration(self) -> Optional[ClassDeclaration]:
        current_class_scope_found = False
        for scope in self.reverse():
            if current_class_scope_found and isinstance(scope, LifetimeScope) and \
                    not isinstance(scope, PartialLifetimeScope):
                return scope.class_declarations[-1]
            elif not current_class_scope_found:
                current_class_scope_found = isinstance(scope, ClassScope)
        return None

    def set_global_import_table(self, import_table: ImportTable):
        global_scope = self.scopes[0]
        if isinstance(global_scope, GlobalScope):
            global_scope.import_table = import_table

    def append_import(self, node: ast.Import):
        for scope in self.reverse():
            if isinstance(scope, ImportScope):
                scope.import_table.append_import(node, not isinstance(scope, GlobalScope))
                return
        assert False

    def append_import_from(self, node: ast.ImportFrom):
        for scope in self.reverse():
            if isinstance(scope, ImportScope):
                scope.import_table.append_import_from(node, not isinstance(scope, GlobalScope))
                return
        assert False

    def handle_import_scope(self):
        if isinstance(self.scopes[-1], ImportScope):
            for declaration in self.scopes[-1].declarations:
                if isinstance(declaration, ImportedDeclaration):
                    declaration.imported_declaration.usages.extend(declaration.usages)

    def persist_current_scope(self):
        # for declaration in self.scopes[-1].declarations:
        #     print(self.get_qualified_name_from_current_scope(declaration))

        if self.persist_symbols > 0:
            return

        self.handle_import_scope()

        import_dto = ImportDTO(self.current_file)

        for var in self.scopes[-1].variable_declarations:
            if isinstance(var, ImportedDeclaration):
                import_dto.add_symbol_import(var.qualified_name, var.module.location, var.position,
                                             var.imported_declaration.qualified_name)
            elif isinstance(var, ModuleVariableDeclaration):
                import_dto.add_module_import(var.qualified_name, var.imported_module_location, var.file_position)
            else:
                self.persistence.persist_variable(var.create_dto())

        for func in self.scopes[-1].function_declarations:
            if isinstance(func, ImportedDeclaration):
                import_dto.add_symbol_import(func.qualified_name, func.module.location, func.position,
                                             func.imported_declaration.qualified_name)
            else:
                self.persistence.persist_function(func.create_dto())

        for cl in self.scopes[-1].class_declarations:
            if isinstance(cl, ImportedDeclaration):
                import_dto.add_symbol_import(cl.qualified_name, cl.module.location, cl.position,
                                             cl.imported_declaration.qualified_name)
            else:
                self.persistence.persist_class(cl.create_dto())

        if len(import_dto.imported_modules) > 0 or len(import_dto.imported_symbols) > 0:
            self.persistence.persist_import(import_dto)

    def get_qualified_scope_name_from_current_scope(self, scopes: Optional[List[Scope]] = None) -> str:
        qualified_name_parts = []
        if scopes is None:
            qualified_name_parts.extend(self.get_qualified_scope_name_from_scopes(self.scopes))
        else:
            qualified_name_parts.extend(self.get_qualified_scope_name_from_scopes(scopes))
        file_name = self.current_file.name
        assert file_name[-3:] == '.py'
        qualified_name_parts.append(file_name[:-3])
        directory = self.current_file.parent
        while True:
            init_file_location = Path(directory).joinpath("__init__.py")
            if init_file_location.exists():
                qualified_name_parts.append(directory.name)
                directory = directory.parent
            else:
                qualified_name_parts.append(str(hash(directory)))
                break
        return '.'.join(reversed(qualified_name_parts))

    def get_qualified_scope_name_from_scopes(self, scopes: List[Scope]):
        qualified_name_parts = []
        for scope in reversed(scopes):
            if isinstance(scope, LifetimeScope) and not isinstance(scope, GlobalScope):  # TODO: ExceptionScope
                qualified_name_parts.append(scope.name)
        return qualified_name_parts

    def get_qualified_name_from_current_scope(self, declaration_name: str, line_num: int) -> str:
        return self.get_qualified_scope_name_from_current_scope() + '.' + declaration_name + ':' + str(line_num)
