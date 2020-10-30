import ast
import typing
from functools import singledispatchmethod
from typing import Optional, Any, Union

from my_ast import built_in_types
from my_ast.base_data import Declaration, ImportedDeclaration, Usage
from my_ast.built_in_functions import get_built_in_function
from my_ast.built_in_operators import get_built_in_operator
from my_ast.built_in_types import Boolean, Dictionary, Set, Tuple, String, Integer, Float, Bytes, \
    EllipsisType, NoneType, Complex, RangeType, BuiltIn, Type, GenericBuiltInType
from my_ast.class_data import ClassDeclaration
from my_ast.common.file_position import FilePosition
from my_ast.common.hashable_list import OrderedHashableList
from my_ast.function_data import FunctionDeclaration
from my_ast.function_symbol_collector_factory import FunctionSymbolCollectorFactory
from my_ast.member_access_collector import MemberAccessCollector, MemberAccessCollectorIterator
from my_ast.preprocessed_file import PreprocessedFile
from my_ast.scope_manager import ScopeManager
from my_ast.symbol_collector_interface import SymbolCollectorBase
from my_ast.type_data import DeclarationType, PlaceholderType, VariablePlaceholderType, FunctionPlaceholderType,\
    InitVariablePlaceholderType
from my_ast.variable_data import VariableDeclaration, FunctionVariable, TypeVariable


class TypeDeduction:
    def __init__(self, symbol_collector: SymbolCollectorBase, scope_manager: ScopeManager,
                 function_symbol_collector_factory: FunctionSymbolCollectorFactory):
        self.scope_manager: ScopeManager = scope_manager
        self.preprocessed_file: Optional[PreprocessedFile] = None
        self.symbol_collector: SymbolCollectorBase = symbol_collector
        self.function_symbol_collector_factory: FunctionSymbolCollectorFactory = function_symbol_collector_factory

    def deduct_type(self, node: ast.AST, preprocessed_file: PreprocessedFile) -> typing.Set:
        self.preprocessed_file = preprocessed_file
        types = self.get_type(node)
        if not isinstance(types, typing.Set):
            return {types}
        return types

    def get_current_type(self, types: typing.Set[DeclarationType])\
            -> typing.Set[Union[ClassDeclaration, BuiltIn, FunctionVariable, TypeVariable, PlaceholderType]]:
        def get_current_type_impl(declaration_type: DeclarationType)\
                -> typing.Set[Union[ClassDeclaration, BuiltIn, FunctionVariable, TypeVariable, PlaceholderType]]:
            if isinstance(declaration_type, PlaceholderType):
                if isinstance(declaration_type, InitVariablePlaceholderType):
                    return {declaration_type}
                declaration = self.scope_manager.get_declaration(declaration_type.name)
                if isinstance(declaration, PlaceholderType) and declaration_type.name == declaration.name:
                    return {declaration}
                return self.get_current_type({declaration})
            elif isinstance(declaration_type, FunctionDeclaration):
                return self.get_current_type(declaration_type.type)
            elif isinstance(declaration_type, (ClassDeclaration, BuiltIn, FunctionVariable, TypeVariable)):
                return {declaration_type}
            elif isinstance(declaration_type, VariableDeclaration):
                return declaration_type.type  # VariableDeclaration.type should only be ClassDeclaration and BuiltIn
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
    def _(self, node: ast.NamedExpr):   # (x := y) eg. if (y := f(x)) is not None: ...
        pass

    @get_type.register
    def _(self, node: ast.BinOp):
        left_operand_type = self.get_current_type(self.deduct_type(node.left, self.preprocessed_file))
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
                print("???")
        return types  # type(left_operand_type.op)

    @get_type.register
    def _(self, node: ast.UnaryOp):
        operand = self.get_current_type(self.deduct_type(node.operand, self.preprocessed_file))
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
                    assert False  # TODO: is this a valid case? (operator on custom class without override it)
            else:
                print("???")
        return types

    @get_type.register
    def _(self, node: ast.Lambda):
        # add node.args to tmp_scope
        return self.get_type(node.body)

    @get_type.register
    def _(self, node: ast.IfExp):
        return {self.get_type(node.body), self.get_type(node.orelse)}

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
        pass  # tmp_var := type(iter) -> type(target)

    @get_type.register
    def _(self, node: ast.Await):
        return self.get_type(node.expr)

    @get_type.register
    def _(self, node: ast.Compare):
        # TODO: handle PlaceHolderType
        left_values = self.deduct_type(node.left, self.preprocessed_file)
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
                    print("???")
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
        return self.get_member_access_type(MemberAccessCollector(node))

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
        return self.get_member_access_type(MemberAccessCollector(node))

    @get_type.register
    def _(self, node: ast.Subscript):
        if isinstance(node.slice, ast.Index):
            if isinstance(self.get_type(node.value), RangeType):  # TODO: get_type -> set(..)
                return Integer()
            else:
                if isinstance(node.value, ast.Name):
                    t = self.deduct_type(node.value, self.preprocessed_file)
                elif isinstance(node.value, ast.Attribute):
                    t = self.deduct_type(node.value, self.preprocessed_file)
                elif isinstance(node.value, ast.Subscript):
                    t = self.deduct_type(node.value, self.preprocessed_file)
                else:
                    assert False
                return set()  # type(node.value->GenericType)
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
        return self.scope_manager.get_declaration(node.id)

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
        element_types = set()
        if isinstance(node, ast.Dict):
            elements = getattr(node, 'values')
        else:
            assert hasattr(node, 'elts')
            elements = getattr(node, 'elts')
        for elem in elements:
            elem_type = self.get_type(elem)
            if elem_type is not None:
                types = self.get_type(elem)
                if isinstance(types, typing.Set):
                    element_types.update(types)
                else:
                    element_types.add(types)
        return element_types

    def get_member_access_type(self, mac: MemberAccessCollector):
        if len(mac.call_list) == 0:
            return set()
        iterator = MemberAccessCollectorIterator(mac)
        declaration = self.scope_manager.get_declaration_from_member_access(iterator)
        if declaration is None or isinstance(declaration, PlaceholderType):
            if iterator.is_iteration_started():
                a = iterator.get_current()
            else:
                a = iterator.get_first()

            # TODO: is empty FilePosition correct?
            if isinstance(a, MemberAccessCollector.MethodData):
                b = [x for x in self.preprocessed_file.preprocessed_functions if x.name == a.name]
                c = [x for x in self.preprocessed_file.class_collector.classes if x.name == a.name]
                for i in b:
                    i.usages.append(Usage(a.name, FilePosition.get_empty_file_position()))
                for i in c:
                    i.usages.append(Usage(a.name, FilePosition.get_empty_file_position()))
            elif isinstance(a, (MemberAccessCollector.AttributeData, MemberAccessCollector.SubscriptData)):
                b = [x for x in self.preprocessed_file.preprocessed_variables if x.name == a.name]
                for i in b:
                    i.usages.append(Usage(a.name, FilePosition.get_empty_file_position()))
            declarations = {declaration}
        else:
            declarations = {declaration}
        if iterator.is_iteration_over() or not iterator.is_iteration_started():
            if not any(isinstance(x, PlaceholderType) for x in declarations) and \
                    any(any(isinstance(x, PlaceholderType) for x in y.type) for y in declarations):
                declarations = self.get_current_type_of_placeholder_function(declarations, mac.call_list[-1])
            return declarations
        prev_member = iterator.get_current()
        next(iterator)
        for member in mac.call_list[iterator.index::-1]:
            if len(declarations) == 0 or all(x is None for x in declarations):
                return set()
            elif all(isinstance(x, PlaceholderType) for x in declarations):
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
                            elif isinstance(declaration_current_type, VariablePlaceholderType):
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
                    else:
                        assert False
        return member_declarations

    def get_member(self, declaration: Declaration, current_member: MemberAccessCollector.MemberData,
                   next_member: MemberAccessCollector.MemberData) \
            -> typing.Set[Union[VariableDeclaration, FunctionDeclaration]]:
        types = self.get_possible_types(declaration)
        declarations = set()
        for t in types:
            if isinstance(t, ClassDeclaration):
                if isinstance(next_member, (MemberAccessCollector.AttributeData, MemberAccessCollector.SubscriptData)):
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

    def get_declaration(self, declaration: MemberAccessCollector.MemberData) \
            -> Optional[Union[VariableDeclaration, FunctionDeclaration]]:
        # TODO: handle global, nonlocal and import declarations
        return self.scope_manager.get_declaration(declaration.name)

    def get_current_type_of_placeholder_function(self, declarations: typing.Set,
                                                 member: MemberAccessCollector.MemberData) \
            -> typing.Set[DeclarationType]:
        types = set()
        for declaration in declarations:
            arguments = OrderedHashableList()
            if isinstance(member, MemberAccessCollector.MethodData):
                for arg in member.arguments:
                    arguments.append(self.deduct_type(arg, self.preprocessed_file))
            declaration_type = self.scope_manager.placeholder_function_declaration_cache. \
                get_functions_return_type(declarations, arguments)
            if declaration_type is not None and len(declaration_type) > 0:
                types.update(declaration_type)
            else:
                if isinstance(declaration, ImportedDeclaration):
                    scope_manager = self.symbol_collector.imported_declaration_scope_map[declaration]
                    func_def = scope_manager.placeholder_function_declaration_cache. \
                        get_function_def(declaration.imported_declaration)
                else:
                    func_def = self.scope_manager.placeholder_function_declaration_cache.get_function_def(declaration)
                # assert func_def is not None   # VariableDeclaration?
                if func_def is None:
                    continue
                # magic
                if len(arguments) == 0:
                    sc = self.function_symbol_collector_factory.get_function_symbol_collector(
                        self.symbol_collector, func_def, [])
                    sc.collect_symbols()
                    types.update(sc.function.type)
                else:
                    for args in self.get_argument_combinations(arguments):
                        sc = self.function_symbol_collector_factory.get_function_symbol_collector(
                            self.symbol_collector, func_def, args)
                        sc.collect_symbols()
                        types.update(sc.function.type)
        return types

    def get_argument_combinations(self, arguments: OrderedHashableList[typing.Set[DeclarationType]]) \
            -> typing.Iterator[OrderedHashableList[DeclarationType]]:
        if len(arguments) == 0:
            return OrderedHashableList()
        for argument in arguments[0]:
            head_argument = OrderedHashableList([argument])
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
