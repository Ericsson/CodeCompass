import ast
from pathlib import PurePath
from typing import List, Optional, Any, Union, Set, TypeVar

from cc_python_parser.built_in_functions import get_built_in_function
from cc_python_parser.built_in_types import GenericType, BuiltIn, List as BuiltInList
from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.common.utils import create_range_from_ast_node
from cc_python_parser.function_symbol_collector_factory import FunctionSymbolCollectorFactory
from cc_python_parser.persistence.persistence import ModelPersistence
from cc_python_parser.placeholder_function_declaration_cache import PlaceholderFunctionDeclarationCache
from cc_python_parser.symbol_collector_interface import SymbolCollectorBase
from cc_python_parser.base_data import Declaration, Usage, ImportedDeclaration
from cc_python_parser.class_data import ClassDeclaration, ImportedClassDeclaration
from cc_python_parser.class_preprocessor import PreprocessedClass
from cc_python_parser.common.parser_tree import ParserTree
from cc_python_parser.import_finder import ImportFinder
from cc_python_parser.import_preprocessor import ImportTable
from cc_python_parser.logger import logger
from cc_python_parser.member_access_collector import MemberAccessCollector, MemberAccessCollectorIterator
from cc_python_parser.preprocessed_file import PreprocessedFile
from cc_python_parser.preprocessed_function import PreprocessedFunction
from cc_python_parser.preprocessed_variable import PreprocessedVariable
from cc_python_parser.scope import ClassScope, FunctionScope, GeneratorScope, LambdaScope, \
    ExceptionScope, ConditionalScope, LoopScope, GlobalScope
from cc_python_parser.function_data import FunctionDeclaration, ImportedFunctionDeclaration, \
    StaticFunctionDeclaration
from cc_python_parser.scope_manager import ScopeManager
from cc_python_parser.symbol_finder import SymbolFinder
from cc_python_parser.type_data import PlaceholderType, InitVariablePlaceholderType, VariablePlaceholderType
from cc_python_parser.type_deduction import TypeDeduction
from cc_python_parser.type_hint_data import TypeHintType
from cc_python_parser.variable_data import VariableDeclaration, ImportedVariableDeclaration, \
    ModuleVariableDeclaration, StaticVariableDeclaration, NonlocalVariableDeclaration, GlobalVariableDeclaration, \
    FunctionVariableDeclaration, TypeVariableDeclaration, MethodVariableDeclaration
from cc_python_parser.common.metrics import metrics


class SymbolCollector(ast.NodeVisitor, SymbolFinder, SymbolCollectorBase):
    def __init__(self, tree: ParserTree, current_file: PurePath, preprocessed_file: PreprocessedFile,
                 import_finder: ImportFinder, persistence: ModelPersistence,
                 function_symbol_collector_factory: FunctionSymbolCollectorFactory):
        SymbolCollectorBase.__init__(self, preprocessed_file, import_finder)
        self.tree = tree
        self.current_file = current_file
        self.scope_manager: ScopeManager = ScopeManager(current_file, import_finder, persistence)
        self.current_function_declaration: List[FunctionDeclaration] = []
        self.current_class_declaration: List[ClassDeclaration] = []
        self.function_symbol_collector_factory = function_symbol_collector_factory
        self.type_deduction = TypeDeduction(self, self.scope_manager,
                                            preprocessed_file, self.function_symbol_collector_factory)

    def collect_symbols(self):
        self.visit(self.tree.root.node)
        self.post_process()

    def generic_visit(self, node: ast.AST) -> Any:
        if type(self) is SymbolCollector:
            metrics.add_ast_count()
        ast.NodeVisitor.generic_visit(self, node)

    def visit_Import(self, node: ast.Import) -> Any:
        metrics.add_import_count()
        modules = ImportTable.convert_ast_import_to_import(node)
        for module in modules:
            if not module.is_module_import():
                continue
            # scope.imports.append((module, self.import_finder.get_global_scope_by_location(module.location)))
            self.scope_manager.append_module_variable_to_current_scope(
                ModuleVariableDeclaration(module.path[-1], module.location,
                                          FilePosition(self.current_file, module.range),
                                          self.import_finder.get_global_scope_by_location(module.location)))
        self.scope_manager.append_import(node)
        self.generic_visit(node)

    def visit_ImportFrom(self, node: ast.ImportFrom) -> Any:
        metrics.add_import_count()
        modules = ImportTable.convert_ast_import_from_to_import(node)
        for module in modules:
            self.scope_manager.append_module_variable_to_current_scope(
                ModuleVariableDeclaration(module.path[-1], module.location,
                                          FilePosition(self.current_file, module.range),
                                          self.import_finder.get_global_scope_by_location(module.location)))
            if not module.is_module_import():
                scope_manager: ScopeManager = self.import_finder.get_scope_manager_by_location(module.location)
                if scope_manager is None:
                    # import from library
                    continue
                # in case of circular import this case is not valid (runtime error) -> must find declaration
                file_position = FilePosition(self.current_file, create_range_from_ast_node(node))
                # TODO: handle transitive imports
                for imported in module.imported:
                    if imported.is_all_imported():
                        for declaration in scope_manager.get_global_scope().declarations:
                            if isinstance(declaration, VariableDeclaration):
                                var = ImportedVariableDeclaration(
                                    declaration.name, file_position, declaration, module)
                                self.scope_manager.append_variable_to_current_scope(var)
                                self.imported_declaration_scope_map[var] = scope_manager
                            elif isinstance(declaration, FunctionDeclaration):
                                func = ImportedFunctionDeclaration(
                                    declaration.name, file_position, declaration, module)
                                self.scope_manager.append_function_to_current_scope(func)
                                self.imported_declaration_scope_map[func] = scope_manager
                            elif isinstance(declaration, ClassDeclaration):
                                c = ImportedClassDeclaration(
                                    declaration.name, file_position, declaration, module)
                                self.scope_manager.append_class_to_current_scope(c)
                                self.imported_declaration_scope_map[c] = scope_manager
                            else:
                                assert False
                    else:
                        # TODO: redefinition?
                        declaration = scope_manager.get_declaration(imported.name)
                        if isinstance(declaration, PlaceholderType):
                            continue
                        elif isinstance(declaration, VariableDeclaration):
                            var = ImportedVariableDeclaration(
                                imported.name if imported.alias is None else imported.alias,
                                file_position, declaration, module)
                            self.scope_manager.append_variable_to_current_scope(var)
                            self.imported_declaration_scope_map[var] = scope_manager
                        elif isinstance(declaration, FunctionDeclaration):
                            func = ImportedFunctionDeclaration(
                                imported.name if imported.alias is None else imported.alias,
                                file_position, declaration, module)
                            self.scope_manager.append_function_to_current_scope(func)
                            self.imported_declaration_scope_map[func] = scope_manager
                        elif isinstance(declaration, ClassDeclaration):
                            c = ImportedClassDeclaration(
                                imported.name if imported.alias is None else imported.alias,
                                file_position, declaration, module)
                            self.scope_manager.append_class_to_current_scope(c)
                            self.imported_declaration_scope_map[c] = scope_manager
                        else:
                            assert False
        self.scope_manager.append_import_from(node)
        self.generic_visit(node)

    def visit_ClassDef(self, node: ast.ClassDef) -> Any:
        metrics.add_class_count()
        base_classes = self.collect_base_classes(node)
        r = create_range_from_ast_node(node)
        qualified_name = self.scope_manager.get_qualified_name_from_current_scope(node.name, r.start_position.line)
        new_class = ClassDeclaration(node.name, qualified_name, FilePosition(self.current_file, r),
                                     base_classes, self.get_documentation(node))
        self.class_def_process_derived_members(new_class, base_classes)
        # TODO: handle preprocessed classes
        self.scope_manager.append_class_to_current_scope(new_class)
        self.current_class_declaration.append(new_class)
        self.scope_manager.persistence.persist_preprocessed_class(new_class.create_dto())

        # TODO: check if has at least 1 parameter? (self)
        # TODO: range could be the __init__
        # self.scope_manager.append_function_to_current_scope(ClassInitDeclaration(new_class))

        def action():
            self.generic_visit(node)
            scope = self.scope_manager.get_current_scope()
            assert isinstance(scope, ClassScope)
            for var in scope.variable_declarations:
                if isinstance(var, StaticVariableDeclaration) and var not in new_class.static_attributes:
                    new_class.static_attributes.append(var)
                elif isinstance(var, VariableDeclaration) and var not in new_class.attributes:
                    new_class.attributes.append(var)
            for func in scope.function_declarations:
                if isinstance(func, StaticFunctionDeclaration):
                    new_class.static_methods.append(func)
                elif isinstance(func, FunctionDeclaration):
                    new_class.methods.append(func)
                else:
                    assert False
            for c in scope.class_declarations:
                new_class.classes.append(c)

        self.scope_manager.with_scope(ClassScope(node.name,
                                                 self.scope_manager.get_qualified_scope_name_from_current_scope(),
                                                 new_class.file_position), action)
        self.class_def_post_process(new_class)
        del self.current_class_declaration[-1]

    def collect_base_classes(self, node: ast.ClassDef) -> List[ClassDeclaration]:
        base_classes = []
        for base_class in node.bases:
            base_class_types = self.type_deduction.get_current_type(
                self.type_deduction.deduct_type(base_class))
            if len(base_class_types) > 1:
                continue  # TODO: eg: standard Lib > ctypes > test > test_byteswap.py > function 'test_struct_struct'
            for base_class_type in base_class_types:
                if isinstance(base_class_type, ClassDeclaration):
                    base_classes.append(base_class_type)
                elif isinstance(base_class_type, TypeVariableDeclaration):
                    if isinstance(base_class_type.reference, ImportedDeclaration):
                        base_classes.append(base_class_type.reference.imported_declaration)
                    elif isinstance(base_class_type.reference, ClassDeclaration):
                        base_classes.append(base_class_type.reference)
                    else:
                        pass    # print('')
                else:
                    pass    # print('')
        return base_classes

    def class_def_process_derived_members(self, cl: ClassDeclaration, base_classes: List[ClassDeclaration]):
        declaration_type = TypeVar('declaration_type', bound=Declaration)

        def collect_derived_members(members: List[List[declaration_type]]) -> List[declaration_type]:
            declarations = []
            for base_class_members in members:
                for member in base_class_members:
                    if all([m.name != member.name for m in declarations]):
                        declarations.append(member)
            return declarations

        cl.attributes.extend(collect_derived_members([d.attributes for d in base_classes]))
        cl.static_attributes.extend(collect_derived_members([d.static_attributes for d in base_classes]))
        cl.methods.extend(collect_derived_members([d.methods for d in base_classes]))
        cl.static_methods.extend(collect_derived_members([d.static_methods for d in base_classes]))
        cl.classes.extend(collect_derived_members([d.classes for d in base_classes]))

    def class_def_post_process(self, cl: ClassDeclaration):
        def remove_hidden_declarations(declarations: List[Declaration]):
            for declaration in reversed(declarations):
                hidden_declarations = [d for d in declarations if d.name == declaration.name and d is not declaration]
                for hidden_declaration in hidden_declarations:
                    declarations.remove(hidden_declaration)

        remove_hidden_declarations(cl.attributes)
        remove_hidden_declarations(cl.static_attributes)
        remove_hidden_declarations(cl.methods)  # TODO: singledispatchmethod
        remove_hidden_declarations(cl.static_methods)
        remove_hidden_declarations(cl.classes)

    def visit_FunctionDef(self, node: ast.FunctionDef) -> Any:
        self.visit_common_function_def(node)

    def visit_AsyncFunctionDef(self, node: ast.AsyncFunctionDef) -> Any:
        self.visit_common_function_def(node)

    def visit_common_function_def(self, node: Union[ast.FunctionDef, ast.AsyncFunctionDef]) -> Any:
        metrics.add_function_count()
        r = create_range_from_ast_node(node)
        qualified_name = self.scope_manager.get_qualified_name_from_current_scope(node.name, r.start_position.line)
        if self.is_static_method(node):
            new_function = StaticFunctionDeclaration(node.name, qualified_name, FilePosition(self.current_file, r),
                                                     [], self.get_documentation(node))
        else:
            new_function = FunctionDeclaration(node.name, qualified_name,
                                               FilePosition(self.current_file, r), [], self.get_documentation(node))
        self.scope_manager.append_function_to_current_scope(new_function)
        self.current_function_declaration.append(new_function)

        # if node.name == '__init__' and isinstance(self.scopes[-1], ClassScope):  # ctor ~ __init__

        def action():
            # self.collect_parameters(node)
            self.process_function_def(node)
            if any(isinstance(x, PlaceholderType) for x in self.current_function_declaration[-1].type):
                is_method = self.scope_manager.is_current_scope_method()
                func_declaration_data = PlaceholderFunctionDeclarationCache.FunctionDeclarationData(node, is_method)
                self.scope_manager.placeholder_function_declaration_cache. \
                    add_function_declaration(new_function, func_declaration_data)

        self.scope_manager.with_scope(FunctionScope(node.name,
                                                    self.scope_manager.get_qualified_scope_name_from_current_scope(),
                                                    new_function.file_position), action)
        del self.current_function_declaration[-1]

    def get_documentation(self, node: Union[ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef]) -> str:
        documentation = ast.get_docstring(node)
        if documentation is None:
            return ""
        else:
            return documentation

    def process_function_def(self, node: Union[ast.FunctionDef, ast.AsyncFunctionDef]) -> None:
        is_init = self.scope_manager.is_current_scope_init()
        is_method = self.scope_manager.is_current_scope_method()

        def get_first_param() -> Optional[ast.arg]:
            if len(node.args.posonlyargs) > 0:
                return node.args.posonlyargs[0]
            if len(node.args.args) > 0:
                return node.args.args[0]
            if node.args.vararg is not None:
                return node.args.vararg
            if len(node.args.kwonlyargs) > 0:
                return node.args.kwonlyargs[0]
            if node.args.kwarg is not None:
                return node.args.kwarg
            return None

        def get_annotation_type(annotation: Optional[ast.expr]) -> Optional[TypeHintType]:
            if annotation is None:
                return None

            hinted_type = None
            if annotation is not None:
                if isinstance(annotation, ast.Constant) and isinstance(annotation.value, str):
                    hinted_type = None
                else:
                    t = self.type_deduction.deduct_type(annotation)
                    current_type = self.type_deduction.get_current_type(t)
                    assert len(current_type) == 0 or len(current_type) == 1
                    if len(current_type) == 1:
                        tt = list(current_type)[0]
                        if isinstance(tt, (ClassDeclaration, BuiltIn)):
                            hinted_type = TypeHintType(tt)
                        elif isinstance(tt, TypeVariableDeclaration):
                            if isinstance(tt.reference, (ClassDeclaration, BuiltIn)):
                                hinted_type = TypeHintType(tt.reference)
                            elif isinstance(tt.reference, ImportedDeclaration):
                                hinted_type = TypeHintType(tt.reference.imported_declaration)
                            else:
                                hinted_type = None
                    else:
                        hinted_type = None
            return hinted_type

        def process_arg(param: Optional[ast.arg]):
            if param is None:
                return

            r = create_range_from_ast_node(param)
            qualified_name = self.scope_manager.get_qualified_name_from_current_scope(param.arg, r.start_position.line)
            new_variable = VariableDeclaration(param.arg, qualified_name, FilePosition(self.current_file, r))

            hinted_type = get_annotation_type(param.annotation)

            if param.arg == 'self' and is_method and get_first_param() is param:
                new_variable.type.add(self.scope_manager.get_current_class_declaration())
            elif is_init:
                if hinted_type is not None:
                    new_variable.type.add(hinted_type)
                else:
                    init_var_placeholder_type = InitVariablePlaceholderType(param.arg)
                    new_variable.type.add(init_var_placeholder_type)
                    self.scope_manager.get_current_class_scope().init_placeholders.append(init_var_placeholder_type)
            else:
                if hinted_type is not None:
                    new_variable.type.add(hinted_type)
                else:
                    new_variable.type.add(VariablePlaceholderType(param.arg))
            self.scope_manager.append_variable_to_current_scope(new_variable)
            self.current_function_declaration[-1].parameters.append(new_variable)

        for arg in node.args.args:              # simple arguments
            process_arg(arg)
        process_arg(node.args.vararg)           # *arg
        for arg in node.args.kwonlyargs:        # arguments between vararg and kwarg
            process_arg(arg)
        process_arg(node.args.kwarg)            # **arg
        for default in node.args.defaults:      # last N args default value
            self.visit(default)
        for default in node.args.kw_defaults:   # default values of kwonlyargs (all)
            if default is not None:
                self.visit(default)
        for arg in node.args.posonlyargs:       # arguments before / "parameter" (after -> args)
            process_arg(arg)

        for decorator in node.decorator_list:
            self.visit(decorator)

        if node.returns is not None:
            self.visit(node.returns)

        for stmt in node.body:
            self.visit(stmt)

        self.current_function_declaration[-1].local_variables.extend(
            self.scope_manager.get_current_scope().variable_declarations)

        return_type = get_annotation_type(node.returns)
        if return_type is not None:
            self.current_function_declaration[-1].type.add(return_type)

    def visit_Return(self, node: ast.Return) -> Any:
        self.visit_common_return(node)

    def visit_Yield(self, node: ast.Yield) -> Any:
        if isinstance(self.scope_manager.get_current_scope(), LambdaScope):
            self.generic_visit(node)
        else:
            self.visit_common_return(node)

    def visit_YieldFrom(self, node: ast.YieldFrom) -> Any:
        self.visit_common_return(node)

    def visit_common_return(self, node: (ast.Return, ast.Yield, ast.YieldFrom)):
        assert isinstance(self.current_function_declaration[-1], FunctionDeclaration)
        if node.value is not None:
            types = self.type_deduction.deduct_type(node)
            if any(t is self.current_function_declaration[-1] for t in types):
                types = {t for t in types if t is not self.current_function_declaration[-1]}
            if len(types) == 0:
                self.current_function_declaration[-1].type.add(VariablePlaceholderType(''))
            else:
                self.current_function_declaration[-1].type.update(types)
        self.generic_visit(node)

    def visit_NamedExpr(self, node: ast.NamedExpr) -> Any:
        self.handle_assignment(node.target, node.value)
        self.generic_visit(node)

    def visit_Assign(self, node: ast.Assign) -> Any:
        for target in node.targets:
            self.handle_assignment(target, node.value)
        self.generic_visit(node)

    def visit_AnnAssign(self, node: ast.AnnAssign) -> Any:
        self.handle_assignment(node.target, node.value)
        self.generic_visit(node)

    def handle_assignment(self, target: ast.AST, value: ast.AST):
        if isinstance(target, (ast.Name, ast.Attribute, ast.Subscript, ast.Starred)):
            self.handle_single_assignment(target, value)
        elif isinstance(target, (ast.List, ast.Tuple)):
            if isinstance(value, (ast.List, ast.Tuple)):
                assert len(target.elts) == len(value.elts)
                for i in range(0, len(target.elts)):
                    self.handle_assignment(target.elts[i], value.elts[i])
            else:
                for t in target.elts:
                    self.handle_assignment(t, value)
        else:
            assert False

    def handle_single_assignment(self, target: ast.AST, value: ast.AST):
        mac = None
        if isinstance(target, (ast.Attribute, ast.Subscript, ast.Starred)):
            mac = MemberAccessCollector(target)
            name = mac.call_list[0].name
        elif isinstance(target, ast.Name):
            name = target.id
        else:
            assert False

        if name == '_':
            return      # TODO

        if value is None:  # variable: type (no value!)
            types = set()
        else:
            types = self.type_deduction.get_current_type(self.type_deduction.deduct_type(value))

        if isinstance(target, ast.Subscript):
            self.handle_subscript_assignment(mac, types)
            return

        r = create_range_from_ast_node(target)
        qualified_name = self.scope_manager.get_qualified_name_from_current_scope(name, r.start_position.line)

        if self.is_static_variable(target):
            new_variable = StaticVariableDeclaration(name, qualified_name, FilePosition(self.current_file, r),
                                                     self.type_deduction.get_current_type(types))
        else:
            new_variable = VariableDeclaration(name, qualified_name, FilePosition(self.current_file, r),
                                               self.type_deduction.get_current_type(types))

        if self.scope_manager.is_current_scope_init() and mac is not None and len(mac.call_list) == 2 and \
                mac.call_list[-1].name == 'self':
            if not self.scope_manager.is_declaration_in_current_class(new_variable.name):
                self.scope_manager.append_variable_to_current_class_scope(new_variable)
                self.scope_manager.get_current_class_declaration().attributes.append(new_variable)
        elif isinstance(target, (ast.Name, ast.Starred)):
            if self.scope_manager.is_inside_conditional_scope() and \
                    not isinstance(declaration := self.scope_manager.get_declaration(name), PlaceholderType):
                if isinstance(value, (ast.Name, ast.Call, ast.Attribute)):
                    d = self.get_declaration(value)
                    dd = self.get_declaration(target)
                if self.has_recursion_in_type(declaration, types):
                    declaration.type = self.fix_recursion_in_type(declaration, types)
                else:
                    declaration.type.update(new_variable.type)
                declaration.type = \
                    {t for t in self.type_deduction.get_current_type(declaration.type)
                     if not isinstance(t, PlaceholderType)}
            else:
                if isinstance(value, (ast.Name, ast.Call, ast.Attribute)):
                    d = self.get_declaration(value)
                    if d is not None:
                        new_variable.type = \
                            {t for t in self.type_deduction.get_current_type(d) if not isinstance(t, PlaceholderType)}
                self.scope_manager.append_variable_to_current_scope(new_variable)
        else:
            pass

    def has_recursion_in_type(self, var: Optional[Union[VariableDeclaration, PlaceholderType]], types: Set):
        if var is None or isinstance(var, PlaceholderType):
            return False
        for t in types:
            if var is t:
                return True
            elif isinstance(t, GenericType) and self.has_recursion_in_type(var, t.types):
                return True
        return False

    def fix_recursion_in_type(self, var: Optional[Union[VariableDeclaration, PlaceholderType]], types: Set) -> Set:
        if var is None or isinstance(var, PlaceholderType):
            return set()
        fixed_types = set()
        for t in types:
            if var is t:
                if isinstance(var, GenericType):
                    fixed_types.update(var.types)
                else:
                    fixed_types.update(var.type)
            elif isinstance(t, GenericType):
                fixed_types.update(self.fix_recursion_in_type(var, t.types))
        return fixed_types

    def handle_subscript_assignment(self, mac: MemberAccessCollector, types: Set):
        name = mac.call_list[0].name
        if name is not None:
            declaration = self.scope_manager.get_variable_declaration(name)
            if not isinstance(declaration, PlaceholderType):
                if self.has_recursion_in_type(declaration, types):
                    declaration.types = self.fix_recursion_in_type(declaration, types)
                elif generics := [d for d in declaration.type if isinstance(d, GenericType)]:
                    for generic in generics:
                        if self.has_recursion_in_type(generic, types):
                            generic.types = self.fix_recursion_in_type(generic, types)
                        else:
                            generic.types.update(types)
        elif len(mac.call_list) == 1 and isinstance(mac.call_list[0], MemberAccessCollector.ReturnValueData) and \
                isinstance(mac.call_list[0].node, ast.BoolOp):
            pass        # TODO: (list_1 or list_2)[i] = x, eg: Lib > idlelib > query.py > showerror function
        else:  # eg.: a[i][j] = k
            depth = 1
            for i in range(1, len(mac.call_list)):
                if not isinstance(mac.call_list[i], MemberAccessCollector.ReturnValueData):
                    depth = i
                    break
            declarations = {self.scope_manager.get_variable_declaration(mac.call_list[depth].name)}
            generic_types = set()
            for i in range(depth, 0, -1):
                for declaration in declarations:
                    if not isinstance(declaration, PlaceholderType) and \
                            (generics := [d for d in declaration.type if isinstance(d, GenericType)]):
                        for generic in generics:
                            if self.has_recursion_in_type(generic_types, generic.types):
                                generic_types = self.fix_recursion_in_type(generic_types, generic.types)
                            else:
                                generic_types.update(generic.types)
                declarations = generic_types.copy()
                generic_types.clear()
            for declaration in declarations:
                if not isinstance(declaration, PlaceholderType) and \
                        (generics := [d for d in declaration.type if isinstance(d, GenericType)]):
                    for generic in generics:
                        if self.has_recursion_in_type(generic, types):
                            generic.types = self.fix_recursion_in_type(generic, types)
                        else:
                            generic.types.update(types)

    def visit_For(self, node: ast.For) -> Any:
        self.visit_common_for(node)

    def visit_AsyncFor(self, node: ast.AsyncFor) -> Any:
        self.visit_common_for(node)

    def visit_common_for(self, node: Union[ast.For, ast.AsyncFor]):
        def handle_loop_variable(types, var):
            declarations = self.get_declaration(var)
            for declaration in declarations:
                if declaration is None or isinstance(declaration, PlaceholderType):
                    assert isinstance(var, ast.Name)
                    if var.id == '_':
                        continue    # TODO
                    r = create_range_from_ast_node(var)
                    qualified_name = self.scope_manager. \
                        get_qualified_name_from_current_scope(var.id, r.start_position.line)
                    new_variable = VariableDeclaration(var.id, qualified_name,
                                                       FilePosition(self.current_file, r), types)
                    self.scope_manager.append_variable_to_current_scope(new_variable)
                else:
                    tt = types
                    if self.has_recursion_in_type(declaration, tt):
                        tt = self.fix_recursion_in_type(declaration, tt)
                    declaration.type = self.type_deduction.get_current_type(tt)
            if len(declarations) == 0:
                self.handle_new_variable(var)

        def handle_loop_target(types, elem):
            if isinstance(elem, (ast.Name, ast.Starred)):
                handle_loop_variable(types, elem)
            elif isinstance(elem, (ast.Tuple, ast.List)):
                for var in elem.elts:
                    handle_loop_target(types, var)
            else:
                assert False

        def action():
            types = self.type_deduction.get_current_type(
                self.type_deduction.deduct_type(node.iter))
            if len(types) == 0:
                self.handle_new_variable(node.target)

            for t in types:
                if isinstance(t, GenericType):
                    handle_loop_target(t.types, node.target)
                elif not isinstance(t, PlaceholderType):
                    iterable = self.get_iterable_type({t})
                    if iterable is not None:
                        next_obj = self.get_next_type(iterable)
                        if next_obj is not None:
                            handle_loop_target(next_obj, node.target)
                        else:
                            self.handle_new_variable(node.target)
                    else:
                        self.handle_new_variable(node.target)
                else:
                    self.handle_new_variable(node.target)
            self.generic_visit(node)

        self.scope_manager.with_scope(LoopScope(), action)

    def handle_new_variable(self, node):
        if isinstance(node, (ast.Name, ast.Starred)):
            r = create_range_from_ast_node(node)
            if isinstance(node, ast.Name):
                id = node.id
            elif isinstance(node, ast.Starred):
                id = node.value.id
            else:
                assert False
            if id == '_':
                return      # TODO
            file_position = FilePosition(self.current_file, r)
            declaration = self.scope_manager.get_declaration(id)
            if not isinstance(declaration, PlaceholderType) and declaration.file_position == file_position:
                return
            qualified_name = self.scope_manager.get_qualified_name_from_current_scope(id, r.start_position.line)
            new_variable = VariableDeclaration(id, qualified_name, file_position)
            self.scope_manager.append_variable_to_current_scope(new_variable)
        elif isinstance(node, (ast.Tuple, ast.List)):
            for var in node.elts:
                self.handle_new_variable(var)
        elif isinstance(node, (ast.Attribute, ast.Subscript)):
            pass        # TODO: handle subscript as loop variable
        else:
            assert False, f"Unhandled type: {type(node)}"

    def get_iterable_type(self, t: Set) -> Optional[Set]:
        current_type = self.type_deduction.get_current_type(t)
        for ct in current_type:
            if isinstance(ct, ClassDeclaration):
                for m in reversed(ct.methods):
                    if m.name == '__iter__':
                        ret = {tt for tt in self.type_deduction.get_current_type({m}) if
                               not isinstance(tt, PlaceholderType)}
                        if len(ret) > 0:
                            return ret
                for m in reversed(ct.static_methods):
                    if m.name == '__iter__':
                        ret = {tt for tt in self.type_deduction.get_current_type({m}) if
                               not isinstance(tt, PlaceholderType)}
                        if len(ret) > 0:
                            return ret
        return None

    def get_next_type(self, t: Set) -> Optional[Set]:
        current_type = self.type_deduction.get_current_type(t)
        for ct in current_type:
            if isinstance(ct, ClassDeclaration):
                for m in reversed(ct.methods):
                    if m.name == '__next__':
                        ret = {tt for tt in self.type_deduction.get_current_type({m}) if
                               not isinstance(tt, PlaceholderType)}
                        if len(ret) > 0:
                            return ret
                for m in reversed(ct.static_methods):
                    if m.name == '__next__':
                        ret = {tt for tt in self.type_deduction.get_current_type({m}) if
                               not isinstance(tt, PlaceholderType)}
                        if len(ret) > 0:
                            return ret
        return None

    def visit_With(self, node: ast.With) -> Any:
        self.common_with_visit(node)

    def visit_AsyncWith(self, node: ast.AsyncWith) -> Any:
        self.common_with_visit(node)

    def common_with_visit(self, node: Union[ast.With, ast.AsyncWith]) -> Any:
        def handle_vars(variables, types):
            if isinstance(variables, ast.Name):
                if variables.id == '_':
                    return  # TODO
                r = create_range_from_ast_node(variables)
                qn = self.scope_manager.get_qualified_name_from_current_scope(variables.id, r.start_position.line)
                new_variable = VariableDeclaration(variables.id, qn, FilePosition(self.current_file, r), types)
                self.scope_manager.append_variable_to_current_scope(new_variable)
            elif isinstance(variables, (ast.List, ast.Tuple)):
                for elem in variables.elts:
                    handle_vars(elem, types)
            elif isinstance(variables, (ast.Attribute, ast.Subscript)):
                pass    # TODO: handle ast.Attribute and ast.Subscript eg: Lib > test > test_buffer.py
            elif isinstance(variables, ast.Starred):
                r = create_range_from_ast_node(variables)
                t = {BuiltInList(types)}
                qn = self.scope_manager.get_qualified_name_from_current_scope(variables.value.id, r.start_position.line)
                new_variable = VariableDeclaration(variables.value.id, qn, FilePosition(self.current_file, r), t)
                self.scope_manager.append_variable_to_current_scope(new_variable)
            else:
                assert False

        def action():
            for with_item in node.items:
                if with_item.optional_vars is not None:
                    types = self.type_deduction.get_current_type(
                                self.type_deduction.deduct_type(with_item.context_expr))
                    handle_vars(with_item.optional_vars, types)
            self.generic_visit(node)

        self.scope_manager.with_scope(LoopScope(), action)

    def visit_ExceptHandler(self, node: ast.ExceptHandler) -> Any:
        def action():
            if node.name is not None and node.name != '_':      # TODO
                r = create_range_from_ast_node(node)
                types = self.type_deduction.deduct_type(node.type)
                qualified_name = self.scope_manager. \
                    get_qualified_name_from_current_scope(node.name, r.start_position.line)
                new_variable = VariableDeclaration(node.name, qualified_name, FilePosition(self.current_file, r), types)
                self.scope_manager.append_exception_to_current_scope(new_variable)
            self.generic_visit(node)

        self.scope_manager.with_scope(ExceptionScope(), action)

    def visit_Global(self, node: ast.Global) -> Any:
        r = create_range_from_ast_node(node)
        for name in node.names:
            reference = self.scope_manager.get_global_variable(name)
            qualified_name = self.scope_manager.get_qualified_name_from_current_scope(name, r.start_position.line)
            if reference is None:
                reference = self.scope_manager.get_global_variable(name)
                qualified_name = self.scope_manager.get_qualified_name_from_current_scope(name, r.start_position.line)
                if reference is None:
                    v = VariableDeclaration(name, qualified_name, FilePosition.get_empty_file_position(),
                                            {VariablePlaceholderType(name)})
                    # TODO: global variable not found, eg: Lib > test > test_dummy_threading.py > global running
                    self.scope_manager.append_variable_to_current_scope(
                        GlobalVariableDeclaration(name, qualified_name, FilePosition(self.current_file, r), v))
                    continue
            self.scope_manager.append_variable_to_current_scope(
                GlobalVariableDeclaration(name, qualified_name, FilePosition(self.current_file, r), reference))
            if reference is not None:
                reference.usages.append(Usage(name, FilePosition(self.current_file, r)))

    def visit_GeneratorExp(self, node: ast.GeneratorExp) -> Any:
        self.scope_manager.with_scope(GeneratorScope(), lambda: self.generic_visit(node))

    def visit_ListComp(self, node: ast.ListComp) -> Any:
        self.scope_manager.with_scope(GeneratorScope(), lambda: self.generic_visit(node))

    def visit_SetComp(self, node: ast.SetComp) -> Any:
        self.scope_manager.with_scope(GeneratorScope(), lambda: self.generic_visit(node))

    def visit_DictComp(self, node: ast.DictComp) -> Any:
        self.scope_manager.with_scope(GeneratorScope(), lambda: self.generic_visit(node))

    def visit_Lambda(self, node: ast.Lambda) -> Any:
        def action():
            for var in node.args.args:
                if var.arg == '_':
                    continue    # TODO
                r = create_range_from_ast_node(var)
                qualified_name = self.scope_manager. \
                    get_qualified_name_from_current_scope(var.arg, r.start_position.line)
                new_variable = VariableDeclaration(var.arg, qualified_name, FilePosition(self.current_file, r))
                self.scope_manager.append_variable_to_current_scope(new_variable)
            self.generic_visit(node)

        self.scope_manager.with_scope(LambdaScope(), action)

    def visit_comprehension(self, node: ast.comprehension) -> Any:
        def handle_comprehension_target(target):
            if isinstance(target, ast.Name):
                if target.id == '_':
                    return
                r = create_range_from_ast_node(target)
                qn = self.scope_manager.get_qualified_name_from_current_scope(target.id, r.start_position.line)
                types = self.type_deduction.get_current_type(self.type_deduction.deduct_type(node.iter))
                new_variable = VariableDeclaration(target.id, qn, FilePosition(self.current_file, r), types)
                self.scope_manager.append_variable_to_current_scope(new_variable)
            elif isinstance(target, ast.Tuple):
                for var in target.elts:
                    handle_comprehension_target(var)
            elif isinstance(target, ast.Subscript):
                pass    # TODO: handle subscript as comprehension target
            else:
                assert False, 'Comprehension target type not handled: ' + type(target)

        handle_comprehension_target(node.target)
        self.generic_visit(node)

    def visit_Nonlocal(self, node: ast.Nonlocal) -> Any:
        r = create_range_from_ast_node(node)
        for name in node.names:
            reference = self.scope_manager.get_nonlocal_variable(name)
            qualified_name = self.scope_manager.get_qualified_name_from_current_scope(name, r.start_position.line)
            if reference is None:
                v = VariableDeclaration(name, qualified_name, FilePosition.get_empty_file_position(),
                                        {VariablePlaceholderType(name)})
                # TODO: nonlocal variable declared after function declaration
                self.scope_manager.append_variable_to_current_scope(
                    NonlocalVariableDeclaration(name, qualified_name, FilePosition(self.current_file, r), v))
                continue
            self.scope_manager.append_variable_to_current_scope(
                NonlocalVariableDeclaration(name, qualified_name, FilePosition(self.current_file, r), reference))
            if reference is not None:
                reference.usages.append(Usage(name, FilePosition(self.current_file, r)))

    def visit_Name(self, node: ast.Name) -> Any:
        if isinstance(node.ctx, ast.Store):
            var = self.get_declaration(node)
            if var is not None and not isinstance(list(var)[0], PlaceholderType):
                other = self.create_var_usage(node.id, node)
                if not list(var)[0].is_same_usage(other):
                    self.append_variable_usage(node.id, node)
            elif node.id == '_':
                pass
            else:
                r = create_range_from_ast_node(node)
                qualified_name = self.scope_manager. \
                    get_qualified_name_from_current_scope(node.id, r.start_position.line)
                self.scope_manager.append_variable_to_current_scope(
                    VariableDeclaration(node.id, qualified_name, FilePosition(self.current_file, r)))
                assert False, 'Assignment target, was not handled at ' + str(r)
        elif isinstance(node.ctx, (ast.Load, ast.Del)):
            self.append_variable_usage(node.id, node)
        else:
            assert False, "Unknown context"
        self.generic_visit(node)

    def visit_Attribute(self, node: ast.Attribute) -> Any:
        self.append_variable_usage(node.attr, node)
        self.generic_visit(node)

    def visit_Call(self, node: ast.Call) -> Any:
        self.visit(node.func)

        for arg in node.args:
            self.visit(arg)
        for keyword in node.keywords:
            self.visit(keyword.value)

        self.append_function_usage(node)

    def visit_Delete(self, node: ast.Delete) -> Any:
        # TODO: remove variable (if not -> only runtime error)
        self.generic_visit(node)

    def visit_If(self, node: ast.If) -> Any:
        # TODO: can be recursive (eg. if..elif..elif..else..), is it a problem?
        self.scope_manager.with_scope(ConditionalScope(), lambda: self.generic_visit(node))

    def append_variable_usage(self, name: str, node: (ast.Name, ast.Attribute, ast.Subscript)) -> None:
        usage = self.create_var_usage(name, node)
        variable_declarations = self.get_declaration(node)
        for var in variable_declarations:
            if var is not None and not isinstance(var, PlaceholderType):
                if isinstance(var, ImportedDeclaration):
                    var.imported_declaration.usages.append(usage)
                else:
                    var.usages.append(usage)
                logger.debug('Var Usage: ' + str(usage) + ' ' + var.get_type_repr())
                return
        # TODO: annotations -> assert
        assert True, "Variable not found: " + usage.name + " (usage: line - " + str(usage.file_position) + ")"

    def append_function_usage(self, node: ast.Call) -> None:
        usage = self.create_function_usage(node)
        mac = self.member_access_collector_type(node)
        is_method_call = (len(mac.call_list) == 2 and
                          isinstance(mac.call_list[1], MemberAccessCollector.AttributeData) and
                          mac.call_list[1].name == 'self')
        if not (len(mac.call_list) == 1 or is_method_call):
            # functions from another module and modules
            # return  # TODO: handle these cases (type deduction)
            pass
        function_declarations = self.get_declaration(node)
        if function_declarations is not None:
            for func in function_declarations:
                if func is not None and not isinstance(func, PlaceholderType):
                    if isinstance(func, ImportedDeclaration):
                        func.imported_declaration.usages.append(usage)
                    else:
                        func.usages.append(usage)
                    logger.debug('Func Usage: ' + str(usage) + ' ' + func.get_type_repr())
                    return
        if self.is_builtin(usage.name):
            self.append_builtin_usage(usage)
            return
        # TODO: imported functions/methods from libraries not in the project (builtins!)
        assert True, "Function not found: " + usage.name + " (usage: start position - " + str(usage.file_position) + ")"

    def is_builtin(self, name: str) -> bool:
        return get_built_in_function(name) is not None

    def append_builtin_usage(self, usage: Usage):
        bif = get_built_in_function(usage.name)
        if bif is not None:
            bif.usages.append(usage)

    def get_declaration(self, node: (ast.Name, ast.Attribute, ast.Call, ast.Subscript)) -> Optional[Set[Declaration]]:
        if isinstance(node, ast.Name):
            declaration = self.scope_manager.get_declaration(node.id)
            if declaration is None:
                return None
            else:
                return {declaration}

        mac = MemberAccessCollector(node)
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Name) and len(mac.call_list) == 1:
            declaration = self.scope_manager.get_declaration(node.func.id)
            if declaration is None or isinstance(declaration, PlaceholderType):
                bif = get_built_in_function(node.func.id)
                if bif is None:
                    return None
                else:
                    return {bif}
            else:
                return {declaration}
        elif isinstance(node, ast.Call) and len(mac.call_list) == 1 and not isinstance(node.func, ast.Subscript):
            pass    # print("NEED FIX: () operator called on lambda, operator or await")

        mac = MemberAccessCollector(node)
        last = mac.call_list.pop(0)
        types = self.type_deduction.get_member_access_type(mac)
        declarations = set()

        for d in types:
            if isinstance(d, ModuleVariableDeclaration):
                if isinstance(d.imported_module, GlobalScope):
                    declaration = self.scope_manager.get_declaration_from_scope(last.name, d.imported_module)
                    if declaration is not None:
                        declarations.add(declaration)
                elif isinstance(d.imported_module, PreprocessedFile):
                    pass
                elif d.imported_module is None:
                    pass
                else:
                    assert False
            elif isinstance(d, (VariableDeclaration, FunctionDeclaration, ClassDeclaration)):
                if isinstance(d, (TypeVariableDeclaration, FunctionVariableDeclaration)):
                    current_types = self.type_deduction.get_current_type({d.reference})
                else:
                    current_types = self.type_deduction.get_current_type(d.type)
                for t in current_types:
                    if isinstance(t, ClassDeclaration):
                        if t is self.scope_manager.get_current_class_declaration():
                            declaration = self.scope_manager.get_declaration_from_current_class(last.name)
                            if isinstance(declaration, PlaceholderType):
                                for cl in reversed(self.preprocessed_file.class_collector.classes):
                                    if not cl.file_position == t.file_position:
                                        continue
                                    for a in reversed(cl.attributes):
                                        if a.name == last.name:
                                            declarations.add(a)
                                    for m in reversed(cl.methods):
                                        if m.name == last.name:
                                            if isinstance(last, MemberAccessCollector.AttributeData):
                                                declarations.add(MethodVariableDeclaration(m, t))
                                            else:
                                                declarations.add(m)
                            else:
                                if isinstance(declaration, FunctionDeclaration) and \
                                        isinstance(last, MemberAccessCollector.AttributeData):
                                    declarations.add(MethodVariableDeclaration(declaration, t))
                                else:
                                    declarations.add(declaration)
                        elif isinstance(last,
                                        (MemberAccessCollector.AttributeData, MemberAccessCollector.SubscriptData)):
                            for a in t.attributes:
                                if a.name == last.name:
                                    declarations.add(a)
                            for a in t.static_attributes:
                                if a.name == last.name:
                                    declarations.add(a)
                        elif isinstance(last, MemberAccessCollector.MethodData):
                            for m in t.methods:
                                if m.name == last.name:
                                    declarations.add(m)
                            for m in t.static_methods:
                                if m.name == last.name:
                                    declarations.add(m)
                    else:
                        pass    # print("NEED FIX: BuiltIn, InitVariablePlaceholderType, VariablePlaceholderType")
            elif isinstance(d, (PlaceholderType, BuiltIn)):
                pass
            else:
                assert False

        return declarations

    # TODO: change PlaceholderType-s, to actual type
    # TODO: declaration is None -> what if not declared in global scope (eg. class member) - pass preprocessed 'scope'?
    def post_process(self) -> None:
        def post_process_variable(var: PreprocessedVariable, class_declaration: Optional[ClassDeclaration]) -> None:
            var_declaration = None
            if class_declaration is None:
                var_declaration = self.scope_manager.get_global_scope().get_variable_by_name(var.name)
            else:
                for a in reversed(class_declaration.attributes):
                    if a.name == var.name:
                        var_declaration = a
                        break
                for a in reversed(class_declaration.static_attributes):
                    if a.name == var.name:
                        var_declaration = a
                        break
            if var_declaration is None:
                return
            var_declaration.usages.extend(var.usages)
            for type_usage in var.type_usages:
                type_usage.type.update(var_declaration.type)

        def post_process_function(func: PreprocessedFunction, class_declaration: Optional[ClassDeclaration]) -> None:
            func_declaration = None
            if class_declaration is None:
                func_declaration = self.scope_manager.get_global_scope().get_function_by_name(func.name)
            else:
                for m in reversed(class_declaration.methods):
                    if m.name == func.name:
                        func_declaration = m
                        break
                for m in reversed(class_declaration.static_methods):
                    if m.name == func.name:
                        func_declaration = m
                        break
            if func_declaration is None:
                return
            func_declaration.usages.extend(func.usages)
            for type_usage in func.type_usages:
                type_usage.type.update(func_declaration.type)

        def post_process_class(pcl: PreprocessedClass, class_declaration: Optional[ClassDeclaration]) -> None:
            current_class_declaration = None
            if class_declaration is None:
                current_class_declaration = self.scope_manager.get_global_scope().get_class_by_name(pcl.name)
            else:
                for cl in reversed(class_declaration.classes):
                    if c.name == cl.name:
                        current_class_declaration = c
                        break

            if current_class_declaration is None:
                return

            for cv in pcl.attributes:
                post_process_variable(cv, current_class_declaration)
            for cf in pcl.methods:
                post_process_function(cf, current_class_declaration)
            for cc in pcl.classes:
                post_process_class(cc, current_class_declaration)

            for type_usage in pcl.type_usages:
                type_usage.type.add(current_class_declaration)

        for v in self.preprocessed_file.preprocessed_variables:
            post_process_variable(v, None)
        for f in self.preprocessed_file.preprocessed_functions:
            post_process_function(f, None)
        for c in self.preprocessed_file.class_collector.classes:
            post_process_class(c, None)

    def create_var_usage(self, name: str, param: ast.expr) -> Usage:
        r = create_range_from_ast_node(param)
        return Usage(name, FilePosition(self.current_file, r))

    def create_function_usage(self, func: ast.Call) -> Usage:
        name = ''
        h = ['func']
        while True:
            if eval('hasattr(' + '.'.join(h) + ', "value")'):
                if eval('isinstance(' + ".".join(h) + ', str)'):
                    break
                else:
                    h.append('value')
            elif eval('hasattr(' + '.'.join(h) + ', "func")'):
                h.append('func')
            else:
                break

        n = []
        for i in range(len(h), 1, -1):
            if eval('hasattr(' + '.'.join(h[:i]) + ', "id")'):
                id_attr = "id"
            elif eval('hasattr(' + '.'.join(h[:i]) + ', "attr")'):
                id_attr = "attr"
            else:
                continue
            n.append(eval("getattr(" + '.'.join(h[:i]) + ", '" + id_attr + "')"))

        # asdf = '.'.join(n)

        mac = MemberAccessCollector(func)

        if hasattr(func.func, 'id'):
            name = func.func.id
        elif hasattr(func.func, 'attr'):
            name = func.func.attr
        else:
            # print("NEED FIX: usage when () operator called on return value")
            assert True  # call on return value (eg. f()(), g[i]())

        r = create_range_from_ast_node(func)
        return Usage(name, FilePosition(self.current_file, r))

    @staticmethod
    def is_static_method(node: Union[ast.FunctionDef, ast.AsyncFunctionDef]) -> bool:
        for decorator in node.decorator_list:
            if isinstance(decorator, ast.Name) and decorator.id == 'staticmethod':
                return True
        return False

    def is_static_variable(self, node: ast.AST) -> bool:
        return isinstance(self.scope_manager.get_current_scope(), ClassScope) and isinstance(node, ast.Name)
