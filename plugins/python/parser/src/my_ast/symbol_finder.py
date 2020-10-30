from typing import Optional
from abc import ABC, abstractmethod

from my_ast.base_data import Declaration


class SymbolFinder(ABC):
    @abstractmethod
    def get_declaration(self, name: str) -> Optional[Declaration]:
        pass
