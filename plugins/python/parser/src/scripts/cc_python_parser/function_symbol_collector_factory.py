import ast
from abc import ABC, abstractmethod
from typing import Union, List, Optional, Tuple

from cc_python_parser.symbol_collector_interface import SymbolCollectorBase, IFunctionSymbolCollector
from cc_python_parser.type_data import DeclarationType


class FunctionSymbolCollectorFactory(ABC):
    @abstractmethod
    def get_function_symbol_collector(self, symbol_collector: SymbolCollectorBase,
                                      func_def: Union[ast.FunctionDef, ast.AsyncFunctionDef],
                                      arguments: List[Tuple[DeclarationType, Optional[str]]])\
            -> IFunctionSymbolCollector:
        pass
