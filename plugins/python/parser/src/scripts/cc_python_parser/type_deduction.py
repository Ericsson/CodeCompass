import ast
import typing
from functools import singledispatchmethod
from typing import Optional, Any, Union

from cc_python_parser import built_in_types
from cc_python_parser.base_data import Declaration, ImportedDeclaration, Usage
from cc_python_parser.built_in_functions import get_built_in_function
from cc_python_parser.built_in_operators import get_built_in_operator
from cc_python_parser.built_in_types import Boolean, Dictionary, Set, Tuple, String, Integer, Float, Bytes, \
    EllipsisType, NoneType, Complex, RangeType, BuiltIn, Type, GenericBuiltInType, NotImplementedType, \
    get_built_in_type, GenericType, Generator
from cc_python_parser.class_data import ClassDeclaration
from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.common.hashable_list import OrderedHashableList
from cc_python_parser.common.utils import create_range_from_ast_node
from cc_python_parser.function_data import FunctionDeclaration
from cc_python_parser.function_symbol_collector_factory import FunctionSymbolCollectorFactory
from cc_python_parser.member_access_collector import MemberAccessCollector, MemberAccessCollectorIterator
from cc_python_parser.preprocessed_data import PreprocessedDeclaration
from cc_python_parser.preprocessed_file import PreprocessedFile
from cc_python_parser.scope_manager import ScopeManager
from cc_python_parser.symbol_collector_interface import SymbolCollectorBase
from cc_python_parser.type_data import DeclarationType, PlaceholderType, VariablePlaceholderType, FunctionPlaceholderType, \
    InitVariablePlaceholderType
from cc_python_parser.type_hint_data import TypeHintType
from cc_python_parser.variable_data import VariableDeclaration, FunctionVariableDeclaration, TypeVariableDeclaration, \
    MethodVariableDeclaration


class TypeDeduction:
    def __init__(self, symbol_collector: SymbolCollectorBase, scope_manager: ScopeManager,
                 preprocessed_file: PreprocessedFile,
                 function_symbol_collector_factory: FunctionSymbolCollectorFactory):
        self.scope_manager: ScopeManager = scope_manager
        self.preprocessed_file: PreprocessedFile = preprocessed_file
        self.symbol_collector: SymbolCollectorBase = symbol_collector
        self.function_symbol_collector_factory: FunctionSymbolCollectorFactory = function_symbol_collector_factory
        self.container_recursion_counter: int = 0
        self.container_max_recursion: int = 5

    def deduct_type(self, node: ast.AST) -> typing.Set:
        types = self.get_type(node)
        if not isinstance(types, typing.Set):
            return {types}
        return types

    def get_current_type(self, types: typing.Set[DeclarationType]) \
            -> typing.Set[Union[
                ClassDeclaration, BuiltIn, FunctionVariableDeclaration, TypeVariableDeclaration, PlaceholderType]]:
        def get_current_type_impl(declaration_type: DeclarationType) \
                -> typing.Set[Union[
                    ClassDeclaration, BuiltIn, FunctionVariableDeclaration, TypeVariableDeclaration, PlaceholderType]]:
            if isinstance(declaration_type, PlaceholderType):
                if isinstance(declaration_type, InitVariablePlaceholderType):
                    return {declaration_type}
                declaration = self.scope_manager.get_declaration(declaration_type.name)
                if isinstance(declaration, PlaceholderType) and declaration_type.name == declaration.name:
                    return {declaration}
                if declaration in types:
                    return set()
                tt = {d for d in declaration.type if d not in types}
                return self.get_current_type(tt)
            elif isinstance(declaration_type, FunctionDeclaration):
                return self.get_current_type(declaration_type.type)
            elif isinstance(declaration_type,
                            (ClassDeclaration, BuiltIn, FunctionVariableDeclaration, TypeVariableDeclaration)):
                return {declaration_type}
            elif isinstance(declaration_type, VariableDeclaration):
                return self.get_current_type(declaration_type.type)
            elif isinstance(declaration_type, TypeHintType):
                return {declaration_type.hinted_type}
            elif isinstance(declaration_type, PreprocessedDeclaration):
                return {declaration_type}
            else:
                assert False

        current_types = set()
        for t in types:
            if t is None:
                continue
            current_types.update(get_current_type_impl(t))
        return current_types

    @singledispatchmethod
    def get_type(self, node: ast.AST) -> Any:
        assert False, "Unknown type: " + str(type(node))

    @get_type.register
    def _(self, node: ast.BoolOp):
        return Boolean()

    @get_type.register
    def _(self, node: ast.NamedExpr):  # (x := y) eg. if (y := f(x)) is not None: ...
        pass

    @get_type.register
    def _(self, node: ast.BinOp):
        left_operand_type = self.get_current_type(self.deduct_type(node.left))
        bio = get_built_in_operator(node.op)
        assert bio is not None
        types = set()
        for t in left_operand_type:
            if isinstance(t, (BuiltIn, PlaceholderType)):
                types.add(t)  # TODO: handle numbers
            elif isinstance(t, ClassDeclaration):
                override = [x for x in t.methods if x.name in bio.get_override()]
                if len(override) > 0:
                    types.add(override[0])
                else:
                    # TODO: check this (eg. base class is BuiltIn)
                    types.add(t)
            else:
                pass    # print("???")
        return types  # type(left_operand_type.op)

    @get_type.register
    def _(self, node: ast.UnaryOp):
        operand = self.get_current_type(self.deduct_type(node.operand))
        bio = get_built_in_operator(node.op)
        assert bio is not None
        types = set()
        for t in operand:
            if isinstance(t, BuiltIn):
                types.add(t)
            elif isinstance(t, ClassDeclaration):
                override = [x for x in t.methods if x.name in bio.get_override()]
                if len(override) > 0:
                    types.add(override[0])
                else:
                    # TODO: preprocessed method
                    types.add(t)
            else:
                pass    # print("???")
        return types

    @get_type.register
    def _(self, node: ast.Lambda):
        # add node.args to tmp_scope
        return self.get_type(node.body)

    @get_type.register
    def _(self, node: ast.IfExp):
        types = self.deduct_type(node.body)
        types.update(self.deduct_type(node.orelse))
        return types

    @get_type.register
    def _(self, node: ast.Dict):
        return Dictionary(self.get_element_types(node))

    @get_type.register
    def _(self, node: ast.Set):
        return Set(self.get_element_types(node))

    @get_type.register
    def _(self, node: ast.ListComp):
        return built_in_types.List()  # generic types?

    @get_type.register
    def _(self, node: ast.SetComp):
        return Set()  # generic types?

    @get_type.register
    def _(self, node: ast.DictComp):
        return Dictionary()  # generic types?

    @get_type.register
    def _(self, node: ast.GeneratorExp):
        g = Generator()
        for generator in node.generators:
            g.types.update(self.deduct_type(generator.iter))
        return g     # tmp_var := type(iter) -> type(target)

    @get_type.register
    def _(self, node: ast.Await):
        return self.get_type(node.value)

    @get_type.register
    def _(self, node: ast.Compare):
        # TODO: handle PlaceHolderType
        left_values = self.deduct_type(node.left)
        types = set()
        assert len(node.comparators) == len(node.ops)
        for i in range(0, len(node.comparators)):
            bio = get_built_in_operator(node.ops[i])
            assert bio is not None
            left_operand_type = self.get_current_type(left_values)
            for t in left_operand_type:
                if isinstance(t, BuiltIn):
                    types.add(Boolean())
                elif isinstance(t, ClassDeclaration):
                    override = [x for x in t.methods if x.name in bio.get_override()]
                    if len(override) > 0:
                        types.add(override[0])
                    else:
                        # TODO: check this (eg. base class is BuiltIn)
                        types.add(t)
                else:
                    pass    # print("???")
            left_values = types
        return left_values

    @get_type.register
    def _(self, node: ast.Call):
        if hasattr(node.func, 'id'):
            # TODO: check if builtin class ctor-s are not hidden by other (custom) functions/classes
            # TODO: no args check (builtin is not hidden + correct code -> should not be a problem)
            built_in_function = get_built_in_function(node.func.id)
            if built_in_function is not None:
                return built_in_function
            elif node.func.id == 'TypeVar':
                return Type()
        return self.get_member_access_type(MemberAccessCollector(node),
                                           FilePosition(self.scope_manager.current_file,
                                                        create_range_from_ast_node(node)))

    @get_type.register
    def _(self, node: ast.FormattedValue):  # {x} in a joined string
        return self.get_type(node.value)  # cannot be on right side without f-string?

    @get_type.register
    def _(self, node: ast.JoinedStr):  # f"... {x} ... {y} ..."
        return String()

    @get_type.register
    def _(self, node: ast.Constant):
        if isinstance(node.value, int):
            if isinstance(node.value, bool):
                return Boolean()
            else:
                return Integer()
        elif isinstance(node.value, float):
            return Float()
        elif isinstance(node.value, bytes):
            return Bytes()
        elif isinstance(node.value, complex):
            return Complex()
        elif isinstance(node.value, str):
            return String()
        elif node.value is None:
            return NoneType()
        elif node.value is Ellipsis:
            return EllipsisType()
        return set()

    @get_type.register
    def _(self, node: ast.Attribute):
        return self.get_member_access_type(MemberAccessCollector(node),
                                           FilePosition(self.scope_manager.current_file,
                                                        create_range_from_ast_node(node)))

    @get_type.register
    def _(self, node: ast.Subscript):
        if isinstance(node.slice, ast.Index):
            if isinstance(self.get_type(node.value), RangeType):  # TODO: get_type -> set(..)
                return Integer()
            else:
                if isinstance(node.value, (ast.Name, ast.Attribute, ast.Subscript, ast.Call, ast.Tuple,
                                           ast.ListComp, ast.List)):
                    t = self.deduct_type(node.value)
                elif isinstance(node.value, (ast.Dict, ast.DictComp)):
                    t = set()
                elif isinstance(node.value, ast.Constant):
                    t = {String()}      # TODO: can be other than str?
                elif isinstance(node.value, (ast.BinOp, ast.UnaryOp)):
                    t = set()
                    operand_types = self.deduct_type(node.value)
                    bio = get_built_in_operator(node.value.op)
                    assert bio is not None
                    for operand_type in self.get_current_type(operand_types):
                        if isinstance(operand_type, ClassDeclaration):
                            override = [x for x in operand_type.methods if x.name in bio.get_override()]
                            if len(override) > 0:
                                t.add(override[0])
                else:
                    assert False, f"Unhandled type: {type(node.value)}"
                generic_types = set()
                subscript_type_sets = [a.type for a in t if not isinstance(a, PlaceholderType)]
                for subscript_types in subscript_type_sets:
                    for subscript_type in subscript_types:
                        if isinstance(subscript_type, GenericType):
                            for generic_type in subscript_type.types:
                                if isinstance(generic_type, ImportedDeclaration):
                                    generic_types.add(generic_type.imported_declaration)
                                else:
                                    generic_types.add(generic_type)
                return generic_types
        elif isinstance(node.slice, (ast.Slice, ast.ExtSlice)):  # ExtSlice -> array type (module arr)
            if isinstance(self.get_type(node.value), RangeType):  # TODO: get_type -> set(..)
                return RangeType()
            else:
                return set()  # type(node.value)[type(node.value->GenericType)]
        return set()  # {type(value)...}

    @get_type.register
    def _(self, node: ast.Starred):  # unpack: * (eg. list, tuple) and ** (eg. dictionary)
        return set()  # could be same as iterator

    @get_type.register
    def _(self, node: ast.Name):
        bit = get_built_in_type(node.id)
        if bit is not None:
            return bit
        if node.id == 'NotImplemented':
            return NotImplementedType()
        elif node.id == 'Ellipsis':
            return EllipsisType()
        return self.scope_manager.get_variable_declaration(node.id)

    @get_type.register
    def _(self, node: ast.List):
        return built_in_types.List(self.get_element_types(node))

    @get_type.register
    def _(self, node: ast.Tuple):
        return Tuple(self.get_element_types(node))

    @get_type.register(ast.Return)
    @get_type.register(ast.Yield)
    @get_type.register(ast.YieldFrom)
    def _(self, node):
        return self.get_type_of_function(node)

    @get_type.register
    def _(self, node: ast.Expr):
        return self.get_type(node.value)

    def get_element_types(self, node: ast.AST):
        # TODO: need better soultion eg: Lib > test > test_parser.py (tuple)
        if self.container_recursion_counter < self.container_recursion_counter:
            return set()
        self.container_recursion_counter += 1
        element_types = set()
        if isinstance(node, ast.Dict):
            elements = getattr(node, 'values')
        else:
            assert hasattr(node, 'elts')
            elements = getattr(node, 'elts')
        for elem in elements:
            elem_type = self.get_type(elem)
            if elem_type is not None:
                if isinstance(elem_type, typing.Set):
                    element_types.update(elem_type)
                else:
                    element_types.add(elem_type)
        self.container_recursion_counter -= 1
        return element_types

    def get_member_access_type(self, mac: MemberAccessCollector, pos: Optional[FilePosition] = None):
        if len(mac.call_list) == 0:
            return set()
        iterator = MemberAccessCollectorIterator(mac)
        declaration = self.scope_manager.get_declaration_from_member_access(iterator)
        if (declaration is None or isinstance(declaration, PlaceholderType)) and pos is not None:
            if iterator.is_iteration_started():
                a = iterator.get_current()
            else:
                a = iterator.get_first()

            # TODO: is empty FilePosition correct?
            if isinstance(a, MemberAccessCollector.MethodData):
                b = [x for x in self.preprocessed_file.preprocessed_functions if x.name == a.name]
                c = [x for x in self.preprocessed_file.class_collector.classes if x.name == a.name]
                for i in b:
                    i.usages.append(Usage(a.name, pos))
                for i in c:
                    i.usages.append(Usage(a.name, pos))
            elif isinstance(a, (MemberAccessCollector.AttributeData, MemberAccessCollector.SubscriptData)):
                b = [x for x in self.preprocessed_file.preprocessed_variables if x.name == a.name]
                for i in b:
                    i.usages.append(Usage(a.name, pos))
        declarations = {declaration}
        if iterator.is_iteration_over() or not iterator.is_iteration_started():
            if not any(isinstance(x, PlaceholderType) for x in declarations) and \
                    any(any(isinstance(x, PlaceholderType) for x in y.type) for y in declarations):
                if any(isinstance(x, FunctionVariableDeclaration) for x in declarations):
                    pass    # print()
                else:
                    declarations = self.get_current_type_of_placeholder_function(declarations, mac.call_list[-1])
            return declarations
        prev_member = iterator.get_current()
        next(iterator)
        for member in mac.call_list[iterator.index::-1]:
            if len(declarations) == 0 or all(x is None for x in declarations) or \
                    all(isinstance(x, PlaceholderType) for x in declarations):
                return set()
            declarations = self.get_member_declarations(declarations, prev_member, member)
            prev_member = member
        return declarations

    def get_member_declarations(self, declarations: typing.Set[DeclarationType],
                                current_member: MemberAccessCollector.MemberData,
                                next_member: MemberAccessCollector.MemberData) \
            -> typing.Set[Union[VariableDeclaration, FunctionDeclaration]]:
        member_declarations = set()
        for declaration in declarations:
            for declaration_type in self.get_current_type(declaration.type):
                if isinstance(declaration_type, PlaceholderType) and isinstance(declaration, FunctionDeclaration):
                    d = self.get_current_type_of_placeholder_function({declaration}, current_member)
                    for dt in d:
                        for declaration_current_type in self.get_possible_types(dt):
                            if isinstance(declaration_current_type, BuiltIn):
                                continue
                            elif isinstance(declaration_current_type, ClassDeclaration):
                                member_declarations.update(
                                    self.get_member(declaration_current_type, current_member, next_member))
                            elif isinstance(declaration_current_type, (PlaceholderType, VariablePlaceholderType)):
                                continue  # TODO: handle this (eg. function parameters)
                            else:
                                assert False
                    # member_declarations.update(d)
                    continue  # TODO: try to get current value of function with the arguments from 'mac'
                for declaration_current_type in self.get_possible_types(declaration_type):
                    if isinstance(declaration_current_type, BuiltIn):
                        self.get_member_of_builtin(declaration_current_type, current_member, next_member)
                    elif isinstance(declaration_current_type, ClassDeclaration):
                        member_declarations.update(self.get_member(
                            declaration_current_type, current_member, next_member))
                    elif isinstance(declaration_current_type, VariablePlaceholderType):
                        continue  # TODO: handle this (eg. function parameters)
                    elif isinstance(declaration_current_type, InitVariablePlaceholderType):
                        continue
                    elif isinstance(declaration_current_type, PlaceholderType):
                        continue
                    else:
                        self.get_possible_types(declaration_type)
                        assert False, f"Unhandled type: {type(declaration_current_type)}"
        return member_declarations

    def get_member(self, declaration: Declaration, current_member: MemberAccessCollector.MemberData,
                   next_member: MemberAccessCollector.MemberData) \
            -> typing.Set[Union[VariableDeclaration, FunctionDeclaration]]:
        types = self.get_possible_types(declaration)
        declarations = set()
        pf = None
        for t in types:
            if isinstance(t, ClassDeclaration):
                if t is self.scope_manager.get_current_class_declaration():
                    pf = self.preprocessed_file
                    pc = None
                    for c in self.preprocessed_file.class_collector.classes:
                        if c.file_position == t.file_position:
                            if isinstance(next_member,
                                          (MemberAccessCollector.AttributeData, MemberAccessCollector.SubscriptData)):
                                for attr in reversed(c.attributes):
                                    if attr.name == next_member.name:
                                        pass
                            elif isinstance(next_member, MemberAccessCollector.MethodData):
                                for method in reversed(c.methods):
                                    if method.name == next_member.name:
                                        arguments = self.collect_current_argument_types(next_member)
                                        # t = self.evaluate_function_def(method.node, call_on, arguments)
                            break
                    if pc is not None:
                        pass
                # else:
                if isinstance(next_member,
                              (MemberAccessCollector.AttributeData, MemberAccessCollector.SubscriptData)):
                    for attr in reversed(t.attributes):
                        if attr.name == next_member.name:
                            declarations.add(attr)
                    for attr in reversed(t.static_attributes):
                        if attr.name == next_member.name:
                            declarations.add(attr)
                elif isinstance(next_member, MemberAccessCollector.MethodData):
                    for method in reversed(t.methods):
                        if method.name == next_member.name:
                            declarations.add(method)
                    for method in reversed(t.static_methods):
                        if method.name == next_member.name:
                            declarations.add(method)
                elif isinstance(next_member, MemberAccessCollector.SubscriptData):
                    # TODO: subscript elements type -> init/call method
                    pass
                elif isinstance(next_member, MemberAccessCollector.LambdaData):
                    # TODO: member.node type -> init/call method
                    pass
                elif isinstance(next_member, MemberAccessCollector.OperatorData):
                    # TODO: member.node type -> init/call method
                    pass
            elif isinstance(t, BuiltIn):
                continue
            else:
                assert False
        return declarations

    def get_member_of_builtin(self, builtin: BuiltIn, current_member: MemberAccessCollector.MemberData,
                              next_member: MemberAccessCollector.MemberData) \
            -> typing.Set[Union[VariableDeclaration, FunctionDeclaration]]:
        if isinstance(builtin, GenericBuiltInType) and isinstance(current_member, MemberAccessCollector.SubscriptData):
            return builtin.types
        return set()

    # evaluate declaration to get type
    def get_possible_types(self, declaration: DeclarationType) -> typing.Set[Union[ClassDeclaration, BuiltIn]]:
        types = set()
        # TODO: handle BuiltIn-s (Module!)
        if isinstance(declaration, BuiltIn):
            return {declaration}
        elif isinstance(declaration, Declaration):
            for t in declaration.type:
                if isinstance(t, (ClassDeclaration, BuiltIn, PlaceholderType)):
                    types.add(t)
                elif isinstance(t, (VariableDeclaration, FunctionDeclaration)):
                    types.update(self.get_possible_types(t))
                else:
                    assert False, "Unknown type: " + str(type(t))
        elif isinstance(declaration, PlaceholderType):
            # TODO: get current value of function or variable
            return self.get_current_type({self.scope_manager.get_declaration(declaration.name)})
        else:
            assert False, "Unknown declaration type: " + str(type(declaration))
        return types

    def get_current_type_of_placeholder_function(self, declarations: typing.Set,
                                                 member: MemberAccessCollector.MemberData) \
            -> typing.Set[DeclarationType]:
        types = set()

        for declaration in declarations:
            if isinstance(declaration, ImportedDeclaration):
                if declaration not in self.symbol_collector.imported_declaration_scope_map:
                    continue    # TODO: fix this, eg: Lib > unittest > test > test_discovery.py > in def restore_listdir
                scope_manager = self.symbol_collector.imported_declaration_scope_map[declaration]
                func_def = scope_manager.placeholder_function_declaration_cache. \
                    get_function_def(declaration.imported_declaration)
            elif isinstance(declaration, FunctionVariableDeclaration):
                func_def = self.scope_manager.placeholder_function_declaration_cache. \
                    get_function_def(declaration.reference)
            else:
                func_def = self.scope_manager.placeholder_function_declaration_cache.get_function_def(declaration)
            # assert func_def is not None   # VariableDeclaration?
            if func_def is None:
                continue

            arguments = OrderedHashableList()
            if isinstance(declaration, MethodVariableDeclaration):
                arguments.append((declaration.self, None))
            elif func_def.is_method:
                arguments.append((self.get_current_type({declaration}), None))
            arguments.extend(self.collect_current_argument_types(member))
            declaration_type = self.scope_manager.placeholder_function_declaration_cache. \
                get_functions_return_type(declarations, arguments)
            if declaration_type is not None and len(declaration_type) > 0:
                types.update(declaration_type)
            else:
                types.update(self.evaluate_function_def(func_def.function, arguments))
        return types

    def collect_current_argument_types(self, method_data: MemberAccessCollector.MemberData) -> OrderedHashableList:
        arguments = OrderedHashableList()
        if isinstance(method_data, MemberAccessCollector.MethodData):
            for arg in method_data.arguments:
                arguments.append((self.deduct_type(arg), None))

            for kw in method_data.keywords:
                arguments.append((self.deduct_type(kw.argument), kw.name))
        return arguments

    def evaluate_function_def(self, function_def: Union[ast.FunctionDef, ast.AsyncFunctionDef],
                              arguments: OrderedHashableList) -> typing.Set[DeclarationType]:
        types = set()
        if len(arguments) == 0:
            sc = self.function_symbol_collector_factory.get_function_symbol_collector(
                self.symbol_collector, function_def, [])
            sc.collect_symbols()
            types.update(sc.function.type)
        else:
            for args in self.get_argument_combinations(arguments):
                sc = self.function_symbol_collector_factory.get_function_symbol_collector(
                    self.symbol_collector, function_def, args)
                sc.collect_symbols()
                types.update(sc.function.type)
        return types

    def get_argument_combinations(self, arguments:
                                  OrderedHashableList[typing.Tuple[typing.Set[DeclarationType], Optional[str]]]) \
            -> typing.Iterator[OrderedHashableList[typing.Tuple[DeclarationType, Optional[str]]]]:
        if len(arguments) == 0:
            return OrderedHashableList()
        for argument in arguments[0][0]:
            head_argument = OrderedHashableList([(argument, arguments[0][1])])
            if len(arguments) == 1:
                yield head_argument
            else:
                for tail_arguments in self.get_argument_combinations(arguments[1::]):
                    yield head_argument + tail_arguments

    def get_type_of_function(self, node: Union[ast.Return, ast.Yield, ast.YieldFrom]):
        if node.value is None:
            return set()

        types = self.get_type(node.value)
        if types is None:
            return set()
        fixed_types = set()
        if isinstance(types, typing.Set):
            fixed_types.update(map(lambda x: self.fix_placeholder(x), types))
        else:
            fixed_types.add(self.fix_placeholder(types))
        return fixed_types

    def fix_placeholder(self, declaration_type: DeclarationType):
        if any(isinstance(x, PlaceholderType) for x in self.get_current_type({declaration_type})):
            if isinstance(declaration_type, VariableDeclaration):
                return VariablePlaceholderType(declaration_type.name)
            elif isinstance(declaration_type, FunctionDeclaration):
                return FunctionPlaceholderType(declaration_type.name)
        return declaration_type
