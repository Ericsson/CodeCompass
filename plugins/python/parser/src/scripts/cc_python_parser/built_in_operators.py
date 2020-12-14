import ast
import inspect
import sys
from abc import ABC, abstractmethod
from typing import Type, List, Optional

from cc_python_parser.built_in_types import Complex, Float, Integer, Boolean


class BuiltInOperator(ABC):
    def __init__(self):
        self.ast_type = self.get_ast_type()
        self.override = self.get_override()

    @staticmethod
    @abstractmethod
    def get_ast_type() -> Type[ast.AST]:
        pass

    @staticmethod
    def get_override() -> List[str]:
        return []

    # @staticmethod
    # @abstractmethod
    # def get_type() -> Type:
    #     pass


class UnaryOperator(BuiltInOperator, ABC):
    pass


class BinaryOperator(BuiltInOperator, ABC):
    pass


class CompareOperator(BuiltInOperator, ABC):
    pass


class BooleanOperator(BuiltInOperator, ABC):
    pass


class AddOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Add

    @staticmethod
    def get_override() -> List[str]:
        return ['__add__']


class SubOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Sub

    @staticmethod
    def get_override() -> List[str]:
        return ['__sub__']


class MultOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Mult

    @staticmethod
    def get_override() -> List[str]:
        return ['__mul__']


class MatMultOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.MatMult

    @staticmethod
    def get_override() -> List[str]:
        return ['__matmul__']


class DivOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Div

    @staticmethod
    def get_override() -> List[str]:
        return ['__truediv__']


class ModOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Mod

    @staticmethod
    def get_override() -> List[str]:
        return ['__mod__']


class PowOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Pow

    @staticmethod
    def get_override() -> List[str]:
        return ['__pow__']


class LShiftOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.LShift

    @staticmethod
    def get_override() -> List[str]:
        return ['__lshift__']


class RShiftOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.RShift

    @staticmethod
    def get_override() -> List[str]:
        return ['__rshift__']


class BitOrOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.BitOr

    @staticmethod
    def get_override() -> List[str]:
        return ['__or__']


class BitXorOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.BitXor

    @staticmethod
    def get_override() -> List[str]:
        return ['__xor__']


class BitAndOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.BitAnd

    @staticmethod
    def get_override() -> List[str]:
        return ['__and__']


class FloorDivOperator(BinaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.FloorDiv

    @staticmethod
    def get_override() -> List[str]:
        return ['__floordiv__']


class AndOperator(BooleanOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.And


class OrOperator(BooleanOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Or


class EqOperator(CompareOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Eq

    @staticmethod
    def get_override() -> List[str]:
        return ['__eq__']


class NotEqOperator(CompareOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.NotEq

    @staticmethod
    def get_override() -> List[str]:
        return ['__ne__']


class LtOperator(CompareOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Lt

    @staticmethod
    def get_override() -> List[str]:
        return ['__lt__']


class LtEOperator(CompareOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.LtE

    @staticmethod
    def get_override() -> List[str]:
        return ['__le__']


class GtOperator(CompareOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Gt

    @staticmethod
    def get_override() -> List[str]:
        return ['__gt__']


class GtEOperator(CompareOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.GtE

    @staticmethod
    def get_override() -> List[str]:
        return ['__ge__']


class IsOperator(CompareOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Is


class IsNotOperator(CompareOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.IsNot


class InOperator(CompareOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.In


class NotInOperator(CompareOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.NotIn


class InvertOperator(UnaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Invert

    @staticmethod
    def get_override() -> List[str]:
        return ['__inv__', '__invert__']


class NotOperator(UnaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.Not

    @staticmethod
    def get_override() -> List[str]:
        return ['__not__']


class UAddOperator(UnaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.UAdd

    @staticmethod
    def get_override() -> List[str]:
        return ['__pos__']


class USubOperator(UnaryOperator):
    @staticmethod
    def get_ast_type() -> Type[ast.AST]:
        return ast.USub

    @staticmethod
    def get_override() -> List[str]:
        return ['__neg__']


built_in_operators = inspect.getmembers(sys.modules[__name__],
                                        lambda x: inspect.isclass(x) and issubclass(x, BuiltInOperator) and
                                                  x is not BuiltInOperator)

number_precedence = [Complex, Float, Integer, Boolean]


def get_built_in_operator(t: ast.AST) -> Optional[BuiltInOperator]:
    bio = [x for x in built_in_operators if type(t) is x[1].get_ast_type()]
    assert len(bio) <= 1
    if len(bio) == 0:
        return None
    return bio[0][1]()
