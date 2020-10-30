import ast
from pathlib import PurePath
from typing import List, Optional, Any, Union, Set, TypeVar

from my_ast.built_in_functions import get_built_in_function
from my_ast.common.file_position import FilePosition
from my_ast.common.utils import create_range_from_ast_node
from my_ast.function_symbol_collector_factory import FunctionSymbolCollectorFactory
from my_ast.symbol_collector_interface import SymbolCollectorBase
from my_ast.base_data import Declaration, Usage
from my_ast.class_data import ClassDeclaration, ImportedClassDeclaration
from my_ast.class_init_declaration import ClassInitDeclaration
from my_ast.class_preprocessor import PreprocessedClass
from my_ast.common.parser_tree import ParserTree
from my_ast.common.position import Range
from my_ast.import_finder import ImportFinder
from my_ast.import_preprocessor import ImportTable
from my_ast.logger import logger
from my_ast.member_access_collector import MemberAccessCollector
from my_ast.preprocessed_file import PreprocessedFile
from my_ast.preprocessed_function import PreprocessedFunction
from my_ast.preprocessed_variable import PreprocessedVariable
from my_ast.scope import Scope, ClassScope, FunctionScope, GeneratorScope, LambdaScope, \
    ExceptionScope, ConditionalScope, LoopScope, GlobalScope
from my_ast.function_data import FunctionDeclaration, ImportedFunctionDeclaration, \
    StaticFunctionDeclaration
from my_ast.scope_manager import ScopeManager
from my_ast.symbol_finder import SymbolFinder
from my_ast.type_data import PlaceholderType, InitVariablePlaceholderType
from my_ast.type_deduction import TypeDeduction, VariablePlaceholderType
from my_ast.variable_data import VariableDeclaration, ImportedVariableDeclaration, \
    ModuleVariableDeclaration, StaticVariableDeclaration, NonlocalVariableDeclaration, GlobalVariableDeclaration, \
    FunctionVariable, TypeVariable


class SymbolCollector(ast.NodeVisitor, SymbolFinder, SymbolCollectorBase):
    def __init__(self, tree: ParserTree, current_file: PurePath, preprocessed_file: PreprocessedFile,
                 import_finder: ImportFinder, function_symbol_collector_factory: FunctionSymbolCollectorFactory):
        SymbolCollectorBase.__init__(self, preprocessed_file, import_finder)
        self.tree = tree
        self.current_file = current_file
        self.scope_manager: ScopeManager = ScopeManager(current_file, import_finder)
        self.scope_manager.set_global_import_table(preprocessed_file.import_table)
        self.current_function_declaration: List[FunctionDeclaration] = []
        self.function_symbol_collector_factory = function_symbol_collector_factory
        self.type_deduction = TypeDeduction(self, self.scope_manager, self.function_symbol_collector_factory)

    def collect_symbols(self):
        self.visit(self.tree.root.node)
        self.post_process()

    def visit_Import(self, node: ast.Import) -> Any:
        modules = ImportTable.convert_ast_import_to_import(node)
        for module in modules:
            if not module.is_module_import():
                continue
            # scope.imports.append((module, self.import_finder.get_global_scope_by_location(module.location)))
            self.scope_manager.append_module_variable_to_current_scope(
                ModuleVariableDeclaration(module.path[-1], module.location,
                                          FilePosition(self.current_file, module.range),
                                          self.import_finder.get_global_scope_by_location(module.location)))
        if len(self.scope_manager.scopes) > 1:
            self.scope_manager.append_import(node)
        self.generic_visit(node)

    def visit_ImportFrom(self, node: ast.ImportFrom) -> Any:
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
                file_position = FilePosition(self.current_file, Range.get_empty_range())
                for imported in module.imported:
                    if imported.is_all_imported():
                        for declaration in scope_manager.get_global_scope().declarations:
                            if isinstance(declaration, VariableDeclaration):
                                var = ImportedVariableDeclaration(
                                    declaration.name, file_position, declaration)
                                self.scope_manager.append_variable_to_current_scope(var)
                                self.imported_declaration_scope_map[var] = scope_manager
                            elif isinstance(declaration, FunctionDeclaration):
                                func = ImportedFunctionDeclaration(
                                    declaration.name, file_position, declaration)
                                self.scope_manager.append_function_to_current_scope(func)
                                self.imported_declaration_scope_map[func] = scope_manager
                            elif isinstance(declaration, ClassDeclaration):
                                c = ImportedClassDeclaration(
                                    declaration.name, file_position, declaration)
                                self.scope_manager.append_class_to_current_scope(c)
                                self.imported_declaration_scope_map[c] = scope_manager
                            else:
                                assert False
                    else:
                        # TODO: redefinition?
                        declaration = scope_manager.get_function_declaration(imported.name)
                        if isinstance(declaration, PlaceholderType):
                            declaration = scope_manager.get_declaration(imported.name)
                            if isinstance(declaration, PlaceholderType):
                                continue
                        elif isinstance(declaration, VariableDeclaration):
                            var = ImportedVariableDeclaration(
                                imported.name if imported.alias is None else imported.alias,
                                file_position, declaration)
                            self.scope_manager.append_variable_to_current_scope(var)
                            self.imported_declaration_scope_map[var] = scope_manager
                        elif isinstance(declaration, FunctionDeclaration):
                            func = ImportedFunctionDeclaration(
                                imported.name if imported.alias is None else imported.alias,
                                file_position, declaration)
                            self.scope_manager.append_function_to_current_scope(func)
                            self.imported_declaration_scope_map[func] = scope_manager
                        elif isinstance(declaration, ClassDeclaration):
                            c = ImportedClassDeclaration(
                                imported.name if imported.alias is None else imported.alias,
                                file_position, declaration)
                            self.scope_manager.append_class_to_current_scope(c)
                            self.imported_declaration_scope_map[c] = scope_manager
                        else:
                            assert False
        if len(self.scope_manager.scopes) > 1:
            self.scope_manager.append_import_from(node)
        self.generic_visit(node)

    def visit_ClassDef(self, node: ast.ClassDef) -> Any:
        base_classes = self.collect_base_classes(node)
        r = create_range_from_ast_node(node)
        qualified_name = self.scope_manager.get_qualified_name_from_current_scope(node.name)
        new_class = ClassDeclaration(node.name, qualified_name, FilePosition(self.current_file, r),
                                     base_classes, self.get_documentation(node))
        self.class_def_process_derived_members(new_class, base_classes)
        # TODO: handle preprocessed classes
        self.scope_manager.get_current_scope().append_class(new_class)
        # TODO: check if has at least 1 parameter? (self)
        # TODO: range could be the __init__
        self.scope_manager.append_function_to_current_scope(
            ClassInitDeclaration(node.name, qualified_name, FilePosition(self.current_file, r), [], new_class))
        self.scope_manager.append_variable_to_current_scope(
            TypeVariable(node.name, qualified_name, FilePosition(self.current_file, r), new_class))

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

        self.scope_manager.with_scope(ClassScope(node.name), action)
        self.class_def_post_process(new_class)

    def collect_base_classes(self, node: ast.ClassDef) -> List[ClassDeclaration]:
        base_classes = []
        for base_class in node.bases:
            # declaration = None
            base_class_types = self.type_deduction.get_current_type(
                self.type_deduction.deduct_type(base_class, self.preprocessed_file))
            # if isinstance(base_class, ast.Name):
            #     declaration = self.scope_manager.get_declaration(base_class.id)
            # elif isinstance(base_class, ast.Subscript):
            #     pass
            #     # declaration = self.scope_manager.get_declaration(base_class.value.id)
            # elif isinstance(base_class, ast.Attribute):
            #     pass
            # else:
            #     assert False
            assert len(base_class_types) <= 1
            for base_class_type in base_class_types:
                if isinstance(base_class_type, ClassDeclaration):
                    base_classes.append(base_class_type)
                elif isinstance(base_class_type, TypeVariable):
                    base_classes.append(base_class_type.reference)
                else:
                    print('')
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
        r = create_range_from_ast_node(node)
        qualified_name = self.scope_manager.get_qualified_name_from_current_scope(node.name)
        if self.is_static_method(node):
            new_function = StaticFunctionDeclaration(node.name, qualified_name, FilePosition(self.current_file, r),
                                                     [], self.get_documentation(node))
        else:
            new_function = FunctionDeclaration(node.name, qualified_name,
                                               FilePosition(self.current_file, r), [], self.get_documentation(node))
        self.scope_manager.append_function_to_current_scope(new_function)
        new_func_var = FunctionVariable(node.name, qualified_name, FilePosition(self.current_file, r), new_function)
        self.scope_manager.append_variable_to_current_scope(new_func_var)
        self.current_function_declaration.append(new_function)

        # if node.name == '__init__' and isinstance(self.scopes[-1], ClassScope):  # ctor ~ __init__

        def action():
            # self.collect_parameters(node)
            self.process_function_def(node)
            if any(isinstance(x, PlaceholderType) for x in self.current_function_declaration[-1].type):
                self.scope_manager.placeholder_function_declaration_cache. \
                    add_function_declaration(new_function, node)

        self.scope_manager.with_scope(FunctionScope(node.name), action)
        del self.current_function_declaration[-1]

    def get_documentation(self, node: Union[ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef]) -> str:
        documentation = ast.get_docstring(node)
        if documentation is None:
            return ""
        else:
            return documentation

    def collect_parameters(self, node: ast.FunctionDef) -> None:
        for param in node.args.args:
            if hasattr(param, 'arg'):  # ast.Name
                r = create_range_from_ast_node(node)
                qualified_name = self.scope_manager.get_qualified_name_from_current_scope(param.arg)
                new_variable = VariableDeclaration(param.arg, qualified_name, FilePosition(self.current_file, r))
                # TODO: first parameter could be anything, not just self + static method check!
                if param.arg == 'self' and self.scope_manager.is_current_scope_method() and node.args.args[0] is param:
                    new_variable.type.add(self.scope_manager.get_current_class_declaration())
                elif node.name == '__init__':
                    init_var_placeholder_type = InitVariablePlaceholderType(param.arg)
                    new_variable.type.add(init_var_placeholder_type)
                    self.scope_manager.get_current_class_scope().init_placeholders.append(init_var_placeholder_type)
                else:
                    new_variable.type.add(VariablePlaceholderType(param.arg))
                self.scope_manager.append_variable_to_current_scope(new_variable)
            else:
                assert False, "Parameter with unknown variable name"

    def process_function_def(self, node: Union[ast.FunctionDef, ast.AsyncFunctionDef]) -> None:
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

        def process_arg(param: Optional[ast.arg]):
            if param is None:
                return
            r = create_range_from_ast_node(arg)
            qualified_name = self.scope_manager.get_qualified_name_from_current_scope(param.arg)
            new_variable = VariableDeclaration(param.arg, qualified_name, FilePosition(self.current_file, r))
            if param.arg == 'self' and is_method and get_first_param() is param:
                new_variable.type.add(self.scope_manager.get_current_class_declaration())
            elif is_init:
                init_var_placeholder_type = InitVariablePlaceholderType(param.arg)
                new_variable.type.add(init_var_placeholder_type)
                self.scope_manager.get_current_class_scope().init_placeholders.append(init_var_placeholder_type)
            else:
                new_variable.type.add(VariablePlaceholderType(param.arg))
            self.scope_manager.append_variable_to_current_scope(new_variable)

        is_init = self.scope_manager.is_current_scope_init()
        is_method = self.scope_manager.is_current_scope_method()
        for arg in node.args.args:              # simple arguments
            process_arg(arg)
        process_arg(node.args.vararg)           # *arg
        for arg in node.args.kwonlyargs:        # arguments between vararg and kwarg
            process_arg(arg)
        process_arg(node.args.kwarg)            # **arg
        for default in node.args.defaults:      # last N args default value
            self.visit(default)
        for default in node.args.kw_defaults:   # default values of kwonlyargs (all)
            self.visit(default)
        for arg in node.args.posonlyargs:       # arguments before / "parameter" (after -> args)
            process_arg(arg)

        for decorator in node.decorator_list:
            self.visit(decorator)

        if node.returns is not None:
            self.visit(node.returns)

        for stmt in node.body:
            self.visit(stmt)

    def visit_Return(self, node: ast.Return) -> Any:
        assert isinstance(self.current_function_declaration[-1], FunctionDeclaration)
        if node.value is not None:
            types = self.type_deduction.deduct_type(node, self.preprocessed_file)
            if len(types) == 0:
                self.current_function_declaration[-1].type.add(VariablePlaceholderType(''))
            else:
                self.current_function_declaration[-1].type.update(types)
        self.generic_visit(node)

    def visit_Yield(self, node: ast.Yield) -> Any:
        assert isinstance(self.current_function_declaration[-1], FunctionDeclaration)
        # TODO: ~Return
        self.current_function_declaration[-1].type.update(
            self.type_deduction.deduct_type(node, self.preprocessed_file))
        self.generic_visit(node)

    def visit_YieldFrom(self, node: ast.YieldFrom) -> Any:
        assert isinstance(self.current_function_declaration[-1], FunctionDeclaration)
        # TODO: ~Return
        self.current_function_declaration[-1].type.update(
            self.type_deduction.deduct_type(node, self.preprocessed_file))
        self.generic_visit(node)

    def visit_NamedExpr(self, node: ast.NamedExpr) -> Any:
        fh = None
        if isinstance(node.target, ast.Attribute):
            fh = MemberAccessCollector(node.target)
            name = fh.call_list[0].name
        elif isinstance(node.target, ast.Name):
            name = node.target.id
        elif isinstance(node.target, (ast.Subscript, ast.Starred, ast.List, ast.Tuple)):
            return
        else:
            assert False

        r = create_range_from_ast_node(node.target)
        types = self.type_deduction.deduct_type(node.value, self.preprocessed_file)
        qualified_name = self.scope_manager.get_qualified_name_from_current_scope(name)
        if self.is_static_variable(node.target):
            new_variable = StaticVariableDeclaration(name, qualified_name, FilePosition(self.current_file, r),
                                                     self.type_deduction.get_current_type(types))
        else:
            new_variable = VariableDeclaration(name, qualified_name, FilePosition(self.current_file, r),
                                               self.type_deduction.get_current_type(types))
        if self.scope_manager.is_current_scope_init() and fh is not None and len(fh.call_list) == 2 and \
                fh.call_list[-1].name == 'self':
            if not self.scope_manager.is_declaration_in_current_class(new_variable.name):
                self.scope_manager.append_variable_to_current_class_scope(new_variable)
                self.scope_manager.get_current_class_declaration().attributes.append(new_variable)
        elif isinstance(node.target, ast.Name):
            if self.scope_manager.is_inside_conditional_scope() and \
                    not isinstance(declaration := self.scope_manager.get_declaration(name), PlaceholderType):
                declaration.type.update(new_variable.type)
            else:
                self.scope_manager.append_variable_to_current_scope(new_variable)
        else:
            print('')
        self.generic_visit(node)

    def visit_Assign(self, node: ast.Assign) -> Any:
        for target in node.targets:
            mac = None
            if isinstance(target, ast.Attribute):
                mac = MemberAccessCollector(target)
                name = mac.call_list[0].name
            elif isinstance(target, ast.Name):
                name = target.id
            elif isinstance(target, (ast.Subscript, ast.Starred, ast.List, ast.Tuple)):
                return
            else:
                assert False

            r = create_range_from_ast_node(target)
            qualified_name = self.scope_manager.get_qualified_name_from_current_scope(name)
            types = self.type_deduction.get_current_type(
                self.type_deduction.deduct_type(node.value, self.preprocessed_file))
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
            elif isinstance(target, ast.Name):
                if self.scope_manager.is_inside_conditional_scope() and \
                        not isinstance(declaration := self.scope_manager.get_declaration(name), PlaceholderType):
                    declaration.type.update(new_variable.type)
                else:
                    self.scope_manager.append_variable_to_current_scope(new_variable)
            else:
                print('')

        self.generic_visit(node)

    def visit_AnnAssign(self, node: ast.AnnAssign) -> Any:
        fh = None
        if isinstance(node.target, ast.Attribute):
            fh = MemberAccessCollector(node.target)
            name = fh.call_list[0].name
        elif isinstance(node.target, ast.Name):
            name = node.target.id
        elif isinstance(node.target, (ast.Subscript, ast.Starred, ast.List, ast.Tuple)):
            return
        else:
            assert False

        r = create_range_from_ast_node(node.target)
        qualified_name = self.scope_manager.get_qualified_name_from_current_scope(name)
        types = self.type_deduction.get_current_type(
            self.type_deduction.deduct_type(node.value, self.preprocessed_file))
        if self.is_static_variable(node.target):
            new_variable = StaticVariableDeclaration(name, qualified_name, FilePosition(self.current_file, r), types)
        else:
            new_variable = VariableDeclaration(name, qualified_name, FilePosition(self.current_file, r), types)
        if self.scope_manager.is_current_scope_init() and fh is not None and \
                len(fh.call_list) == 2 and fh.call_list[-1].name == 'self':
            if not self.scope_manager.is_declaration_in_current_class(new_variable.name):
                self.scope_manager.append_variable_to_current_class_scope(new_variable)  # TODO: use type of self
        elif isinstance(node.target, ast.Name):
            if not self.scope_manager.is_declaration_in_current_scope(new_variable.name):
                self.scope_manager.append_variable_to_current_scope(new_variable)  # TODO: not only in last scope

    def visit_For(self, node: ast.For) -> Any:
        self.visit_common_for(node)

    def visit_AsyncFor(self, node: ast.AsyncFor) -> Any:
        self.visit_common_for(node)

    def visit_common_for(self, node: Union[ast.For, ast.AsyncFor]):
        def action():
            if not isinstance(node.target, ast.Tuple) and \
                    not self.scope_manager.is_declaration_in_current_scope(node.target.id):
                r = create_range_from_ast_node(node.target)
                types = self.type_deduction.deduct_type(node.iter, self.preprocessed_file)
                qn = self.scope_manager.get_qualified_name_from_current_scope(node.target.id)
                new_variable = VariableDeclaration(node.target.id, qn, FilePosition(self.current_file, r), types)
                self.scope_manager.append_variable_to_current_scope(new_variable)
            elif isinstance(node.target, ast.Tuple):
                for var in node.target.elts:
                    r = create_range_from_ast_node(var)
                    # TODO: type
                    types = set()
                    # self.type_deduction.deduct_type(node.iter, self.preprocessed_file.class_collector)
                    qn = self.scope_manager.get_qualified_name_from_current_scope(var.id)
                    new_variable = VariableDeclaration(var.id, qn, FilePosition(self.current_file, r), types)
                    self.scope_manager.append_variable_to_current_scope(new_variable)
            else:
                # types = self.type_deduction.deduct_type(...)
                # get variable
                # add types
                pass
            self.generic_visit(node)

        self.scope_manager.with_scope(LoopScope(), action)

    def visit_With(self, node: ast.With) -> Any:
        self.common_with_visit(node)

    def visit_AsyncWith(self, node: ast.AsyncWith) -> Any:
        self.common_with_visit(node)

    def common_with_visit(self, node: Union[ast.With, ast.AsyncWith]) -> Any:
        def action():
            for with_item in node.items:
                if with_item.optional_vars is not None:
                    r = create_range_from_ast_node(with_item.optional_vars)
                    qn = self.scope_manager.get_qualified_name_from_current_scope(with_item.optional_vars.id)
                    types = self.type_deduction.deduct_type(with_item.context_expr, self.preprocessed_file)
                    new_variable = VariableDeclaration(with_item.optional_vars.id, qn,
                                                       FilePosition(self.current_file, r), types)
                    self.scope_manager.append_variable_to_current_scope(new_variable)
            self.generic_visit(node)

        self.scope_manager.with_scope(LoopScope(), action)

    def visit_ExceptHandler(self, node: ast.ExceptHandler) -> Any:
        def action():
            if node.name is not None:
                r = create_range_from_ast_node(node)
                types = self.type_deduction.deduct_type(node.type, self.preprocessed_file)
                qualified_name = self.scope_manager.get_qualified_name_from_current_scope(node.name)
                new_variable = VariableDeclaration(node.name, qualified_name, FilePosition(self.current_file, r), types)
                self.scope_manager.append_exception_to_current_scope(new_variable)
            self.generic_visit(node)

        self.scope_manager.with_scope(ExceptionScope(), action)

    def visit_Global(self, node: ast.Global) -> Any:
        r = create_range_from_ast_node(node)
        for name in node.names:
            reference = self.scope_manager.get_global_variable(name)
            if reference is None:
                continue
            qualified_name = self.scope_manager.get_qualified_name_from_current_scope(name)
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
                r = create_range_from_ast_node(var)
                qualified_name = self.scope_manager.get_qualified_name_from_current_scope(var.arg)
                new_variable = VariableDeclaration(var.arg, qualified_name, FilePosition(self.current_file, r))
                self.scope_manager.append_variable_to_current_scope(new_variable)
            self.generic_visit(node)

        self.scope_manager.with_scope(LambdaScope(), action)

    def visit_comprehension(self, node: ast.comprehension) -> Any:
        if isinstance(node.target, ast.Name):
            r = create_range_from_ast_node(node.target)
            qn = self.scope_manager.get_qualified_name_from_current_scope(node.target.id)
            types = self.type_deduction.deduct_type(node.iter, self.preprocessed_file)
            new_variable = VariableDeclaration(node.target.id, qn, FilePosition(self.current_file, r), types)
            self.scope_manager.append_variable_to_current_scope(new_variable)
        elif isinstance(node.target, ast.Tuple):
            for var in node.target.elts:
                r = create_range_from_ast_node(var)
                qualified_name = self.scope_manager.get_qualified_name_from_current_scope(var.id)
                types = self.type_deduction.deduct_type(node.iter, self.preprocessed_file)
                new_variable = VariableDeclaration(var.id, qualified_name, FilePosition(self.current_file, r), types)
                self.scope_manager.append_variable_to_current_scope(new_variable)
        else:
            assert False, 'Comprehension target type not handled: ' + type(node.target)
        self.generic_visit(node)

    def visit_Nonlocal(self, node: ast.Nonlocal) -> Any:
        r = create_range_from_ast_node(node)
        for name in node.names:
            reference = self.scope_manager.get_nonlocal_variable(name)
            if reference is None:
                continue
            qualified_name = self.scope_manager.get_qualified_name_from_current_scope(name)
            self.scope_manager.append_variable_to_current_scope(
                NonlocalVariableDeclaration(name, qualified_name, FilePosition(self.current_file, r), reference))
            if reference is not None:
                reference.usages.append(Usage(name, FilePosition(self.current_file, r)))

    def visit_Name(self, node: ast.Name) -> Any:
        if isinstance(node.ctx, ast.Store):
            var = self.get_declaration(node)
            if var is not None:
                other = self.create_var_usage(node.id, node)
                if not list(var)[0].is_same_usage(other):
                    self.append_variable_usage(node.id, node)
            else:
                r = create_range_from_ast_node(node)
                qualified_name = self.scope_manager.get_qualified_name_from_current_scope(node.id)
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
        types = self.type_deduction.deduct_type(node, self.preprocessed_file)
        if len(types) == 0:
            print('ph')
        for t in types:
            from my_ast.type_data import PlaceholderType
            if isinstance(t, PlaceholderType):
                print('ph')
        for t in self.type_deduction.get_current_type(types):
            from my_ast.type_data import PlaceholderType
            if isinstance(t, PlaceholderType):
                print('ph')

        self.generic_visit(node.func)

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

    @staticmethod
    def get_declaration_from_scope(name: str, scope: Scope) -> Optional[Declaration]:
        for var in scope.declarations:
            if var.name == name:
                return var
        return None

    @staticmethod
    def is_declaration_in_scope(name: str, scope: Scope) -> bool:
        for var in scope.declarations:
            if var.name == name:
                return True
        return False

    @staticmethod
    def is_variable_declaration_in_scope(name: str, scope: Scope) -> bool:
        for var in scope.declarations:
            if var.name == name:
                return isinstance(var, VariableDeclaration)
        return False

    @staticmethod
    def is_function_declaration_in_scope(name: str, scope: Scope) -> bool:
        for var in scope.declarations:
            if var.name == name:
                return isinstance(var, FunctionDeclaration)
        return False

    @staticmethod
    def is_class_declaration_in_scope(name: str, scope: Scope) -> bool:
        for var in scope.declarations:
            if var.name == name:
                return isinstance(var, ClassDeclaration)
        return False

    def append_variable_usage(self, name: str, node: (ast.Name, ast.Attribute, ast.Subscript)) -> None:
        usage = self.create_var_usage(name, node)
        variable_declarations = self.get_declaration(node)
        for var in variable_declarations:
            if var is not None and not isinstance(var, PlaceholderType):
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
        for func in function_declarations:
            if func is not None and not isinstance(func, PlaceholderType):
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
            if declaration is None:
                return None
            else:
                return {declaration}
        elif isinstance(node, ast.Call) and len(mac.call_list) == 1 and not isinstance(node.func, ast.Subscript):
            print("NEED FIX: () operator called on lambda, operator or await")

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
            elif isinstance(d, (VariableDeclaration, FunctionDeclaration)):
                if isinstance(d, (TypeVariable, FunctionVariable)):
                    current_types = self.type_deduction.get_current_type({d.reference})
                else:
                    current_types = self.type_deduction.get_current_type(d.type)
                for t in current_types:
                    if isinstance(t, ClassDeclaration):
                        if t is self.scope_manager.get_current_class_declaration():
                            declarations.add(self.scope_manager.get_declaration(last.name))
                        elif isinstance(last,
                                        (MemberAccessCollector.AttributeData, MemberAccessCollector.SubscriptData)):
                            for m in t.attributes:
                                if m.name == last.name:
                                    declarations.add(m)
                            for m in t.static_attributes:
                                if m.name == last.name:
                                    declarations.add(m)
                        elif isinstance(last, MemberAccessCollector.MethodData):
                            for m in t.methods:
                                if m.name == last.name:
                                    declarations.add(m)
                            for m in t.static_methods:
                                if m.name == last.name:
                                    declarations.add(m)
                    else:
                        print("NEED FIX: BuiltIn, InitVariablePlaceholderType, VariablePlaceholderType")
            elif isinstance(d, PlaceholderType):
                pass
            else:
                assert False

        return declarations

    # TODO: change PlaceholderType-s, to actual type
    # TODO: declaration is None -> what if not declared in global scope (eg. class member) - pass preprocessed 'scope'?
    def post_process(self) -> None:
        def post_process_variable(var: PreprocessedVariable) -> None:
            var_declaration = self.scope_manager.get_global_scope().get_variable_by_name(var.name)
            if var_declaration is None:
                return
            var_declaration.usages.extend(var.usages)
            for type_usage in var.type_usages:
                type_usage.type.update(var_declaration.type)

        def post_process_function(func: PreprocessedFunction) -> None:
            func_declaration = self.scope_manager.get_global_scope().get_function_by_name(func.name)
            if func_declaration is None:
                return
            func_declaration.usages.extend(func.usages)
            for type_usage in func.type_usages:
                type_usage.type.update(func_declaration.type)

        def post_process_class(cl: PreprocessedClass) -> None:
            for cv in cl.attributes:
                post_process_variable(cv)
            for cf in cl.methods:
                post_process_function(cf)
            for cc in cl.classes:
                post_process_class(cc)

            class_declaration = self.scope_manager.get_global_scope().get_class_by_name(cl.name)
            if class_declaration is None:
                return
            for type_usage in cl.type_usages:
                type_usage.type.update(class_declaration.type)

        for v in self.preprocessed_file.preprocessed_variables:
            post_process_variable(v)
        for f in self.preprocessed_file.preprocessed_functions:
            post_process_function(f)
        for c in self.preprocessed_file.class_collector.classes:
            post_process_class(c)

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
            print("NEED FIX: usage when () operator called on return value")
            assert True     # call on return value (eg. f()(), g[i]())

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
