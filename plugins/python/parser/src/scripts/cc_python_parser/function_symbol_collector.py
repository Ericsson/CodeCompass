import ast
from typing import Union, Any, List, Optional, Set, Tuple

from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.common.parser_tree import ParserTree
from cc_python_parser.common.utils import create_range_from_ast_node
from cc_python_parser.function_data import FunctionDeclaration
from cc_python_parser.scope import FunctionScope
from cc_python_parser.symbol_collector import SymbolCollector
from cc_python_parser.symbol_collector_interface import IFunctionSymbolCollector
from cc_python_parser.type_data import DeclarationType
from cc_python_parser.type_deduction import TypeDeduction
from cc_python_parser.variable_data import VariableDeclaration


class TemporaryScope(FunctionScope):
    pass


class FunctionSymbolCollector(SymbolCollector, IFunctionSymbolCollector):
    def __init__(self, symbol_collector: SymbolCollector, tree: Union[ast.FunctionDef, ast.AsyncFunctionDef],
                 arguments: List[Tuple[DeclarationType, Optional[str]]]):
        SymbolCollector.__init__(self, ParserTree(tree), symbol_collector.current_file,
                                 symbol_collector.preprocessed_file,
                                 symbol_collector.import_finder,
                                 symbol_collector.scope_manager.persistence,
                                 symbol_collector.function_symbol_collector_factory)
        IFunctionSymbolCollector.__init__(self)
        self.scope_manager = symbol_collector.scope_manager
        self.type_deduction = TypeDeduction(self, self.scope_manager, self.preprocessed_file,
                                            self.function_symbol_collector_factory)
        self.imported_declaration_scope_map = symbol_collector.imported_declaration_scope_map

        self.arguments = arguments
        self.current_function_calls = []
        if isinstance(symbol_collector, FunctionSymbolCollector):
            self.current_function_calls.extend(symbol_collector.current_function_calls)
        if len(symbol_collector.current_function_declaration) > 0:
            self.current_function_calls.append(symbol_collector.current_function_declaration[-1])

    def collect_symbols(self):
        self.scope_manager.lock_persisting()
        SymbolCollector.collect_symbols(self)
        self.scope_manager.unlock_persisting()

    # TODO: handle local functions
    # TODO: circular import + placeholder?
    def visit_common_function_def(self, node: Union[ast.FunctionDef, ast.AsyncFunctionDef]) -> Any:
        r = create_range_from_ast_node(node)
        self.function = FunctionDeclaration(node.name,
                                            self.scope_manager.get_qualified_name_from_current_scope(
                                                node.name, r.start_position.line),
                                            FilePosition(self.current_file, r), [], "")

        for call in reversed(self.current_function_calls):
            if call.name == self.function.name and call.file_position == self.function.file_position:
                return
        # self.scope_manager.append_function_to_current_scope(self.function)
        self.current_function_declaration.append(self.function)

        def action():
            self.collect_parameters(node)
            self.generic_visit(node)

        self.scope_manager.with_scope(FunctionScope(node.name,
                                                    self.scope_manager.get_qualified_scope_name_from_current_scope(),
                                                    FilePosition.get_empty_file_position()), action)
        del self.current_function_declaration[-1]

    def collect_parameters(self, node: Union[ast.FunctionDef, ast.AsyncFunctionDef]) -> None:
        args_with_default = []
        for i in range(0, len(node.args.args)):
            if i < len(node.args.defaults):
                default = node.args.defaults[-(i+1)]
            else:
                default = None
            args_with_default.append((node.args.args[-(i+1)], default))

        arg_idx = 0
        for param in reversed(args_with_default):
            r = create_range_from_ast_node(node)
            qualified_name = self.scope_manager.\
                get_qualified_name_from_current_scope(param[0].arg, r.start_position.line)
            new_variable = VariableDeclaration(param[0].arg, qualified_name, FilePosition(self.current_file, r))
            if param[0].arg == 'self' and self.scope_manager.is_current_scope_method() and \
                    node.args.args[0] is param[0]:
                new_variable.type.add(self.scope_manager.get_current_class_declaration())
            else:
                # has kw -> has arg -> default
                keyword = [k for k in self.arguments if k[1] == param[0].arg]
                if len(keyword) == 0:
                    if arg_idx < len(self.arguments):
                        new_variable.type.update(self.type_deduction.get_current_type({self.arguments[arg_idx][0]}))
                        arg_idx += 1
                    elif param[1] is not None:
                        new_variable.type.update(self.type_deduction.get_current_type(
                            self.type_deduction.deduct_type(param[1])))
                elif len(keyword) == 1:
                    new_variable.type.update(self.type_deduction.get_current_type({keyword[0][0]}))
                else:
                    assert False
            self.scope_manager.append_variable_to_current_scope(new_variable)
