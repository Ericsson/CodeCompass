import ast
from abc import ABC, abstractmethod
from typing import Union, List

from my_ast.symbol_collector_interface import SymbolCollectorBase, IFunctionSymbolCollector
from my_ast.type_data import DeclarationType


class FunctionSymbolCollectorFactory(ABC):
    @abstractmethod
    def get_function_symbol_collector(self, symbol_collector: SymbolCollectorBase,
                                      func_def: Union[ast.FunctionDef, ast.AsyncFunctionDef],
                                      arguments: List[DeclarationType]) -> IFunctionSymbolCollector:
        pass
