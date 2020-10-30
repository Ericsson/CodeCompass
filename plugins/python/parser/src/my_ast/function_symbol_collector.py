import ast
from typing import Union, Any, List

from my_ast.common.file_position import FilePosition
from my_ast.common.parser_tree import ParserTree
from my_ast.common.utils import create_range_from_ast_node
from my_ast.function_data import FunctionDeclaration
from my_ast.scope import FunctionScope
from my_ast.symbol_collector import SymbolCollector
from my_ast.symbol_collector_interface import IFunctionSymbolCollector
from my_ast.type_data import DeclarationType
from my_ast.type_deduction import TypeDeduction
from my_ast.variable_data import VariableDeclaration


class TemporaryScope(FunctionScope):
    pass


class FunctionSymbolCollector(SymbolCollector, IFunctionSymbolCollector):
    def __init__(self, symbol_collector: SymbolCollector, tree: Union[ast.FunctionDef, ast.AsyncFunctionDef],
                 arguments: List[DeclarationType]):
        SymbolCollector.__init__(self, ParserTree(tree), symbol_collector.current_file,
                                 symbol_collector.preprocessed_file,
                                 symbol_collector.import_finder,
                                 symbol_collector.function_symbol_collector_factory)
        IFunctionSymbolCollector.__init__(self)
        self.scope_manager = symbol_collector.scope_manager
        self.type_deduction = TypeDeduction(self, self.scope_manager, self.function_symbol_collector_factory)
        self.arguments = arguments

    # TODO: handle local functions
    # TODO: circular import + placeholder?
    def visit_common_function_def(self, node: Union[ast.FunctionDef, ast.AsyncFunctionDef]) -> Any:
        r = create_range_from_ast_node(node)
        self.function = FunctionDeclaration(node.name,
                                            self.scope_manager.get_qualified_name_from_current_scope(node.name),
                                            FilePosition(self.current_file, r), [], "")
        # self.scope_manager.append_function_to_current_scope(self.function)
        self.current_function_declaration.append(self.function)

        def action():
            self.collect_parameters(node)
            self.generic_visit(node)

        self.scope_manager.with_scope(FunctionScope(node.name), action)
        del self.current_function_declaration[-1]

    def collect_parameters(self, node: ast.FunctionDef) -> None:
        arg_idx = 0
        for param in node.args.args:
            if hasattr(param, 'arg'):  # ast.Name
                r = create_range_from_ast_node(node)
                new_variable = VariableDeclaration(param.arg,
                                                   self.scope_manager.get_qualified_name_from_current_scope(param.arg),
                                                   FilePosition(self.current_file, r))
                if param.arg == 'self' and self.scope_manager.is_current_scope_method() and node.args.args[0] is param:
                    new_variable.type.add(self.scope_manager.get_current_class_declaration())
                else:
                    new_variable.type.update(self.type_deduction.get_current_type({self.arguments[arg_idx]}))
                    arg_idx += 1
                self.scope_manager.append_variable_to_current_scope(new_variable)
            else:
                assert False, "Parameter with unknown variable name"
