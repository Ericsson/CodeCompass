import inspect
import sys
from abc import ABC, abstractmethod
from typing import Optional, Set, Union, List

from cc_python_parser import built_in_types
from cc_python_parser.built_in_types import Boolean, Complex, Dictionary, Float, FrozenSet, Integer, File, Slice, Tuple, \
    Object, MemoryView, String, Bytes, ByteArray, Type, RangeType
from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.function_data import FunctionDeclaration
from cc_python_parser.type_data import DeclarationType


class BuiltInFunction(FunctionDeclaration, ABC):
    def __init__(self):
        file_position = FilePosition.get_empty_file_position()
        qualified_name = "builtins.function." + self.get_name()
        super().__init__(self.get_name(), qualified_name, file_position, [], "", self.get_type())
        if self.get_override() is None:
            self.override = []
        else:
            self.override: List[str] = self.get_override()

    @staticmethod
    @abstractmethod
    def get_name() -> str:
        pass

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return None

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return None

    def get_type_repr(self) -> str:
        if self.get_type() is None:
            return ""
        return ','.join([x.get_type_repr() for x in self.get_type()])


class FunctionDecorator(BuiltInFunction, ABC):
    pass


class AbsFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'abs'

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return ['__abs__']


class AllFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'all'


class AnyFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'any'


class AsciiFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'ascii'


# returns binary string
class BinFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'bin'


class BoolFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'bool'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Boolean()}


class BreakpointFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'breakpoint'


class BytearrayFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'bytearray'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {ByteArray()}


class BytesFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'bytes'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Bytes()}


class CallableFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'callable'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Boolean()}


class ChrFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'chr'


class ClassmethodFunction(FunctionDecorator):
    @staticmethod
    def get_name() -> str:
        return 'classmethod'


class CompileFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'compile'


class ComplexFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'complex'

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return ['__complex__', '__float__', '__index__']

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Complex()}


class DelattrFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'delattr'


class DictFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'dict'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Dictionary()}


class DirFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'dir'

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return ['__dir__']


class DivmodFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'divmod'


# returns enumerate object
class EnumerateFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'enumerate'


class EvalFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'eval'


class ExecFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'exec'


class FilterFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'filter'


class FloatFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'float'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Float()}


class FormatFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'format'

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return ['__format__']


class FrozensetFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'frozenset'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {FrozenSet()}


class GetattrFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'getattr'


class GlobalsFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'globals'


class HasattrFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'hasattr'


class HashFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'hash'

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return ['__hash__']


class HelpFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'help'


class HexFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'hex'


class IdFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'id'


class InputFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'input'


class IntFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'int'

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return ['__init__', '__index__', '__truncate__']

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Integer()}


class IsinstanceFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'isinstance'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Boolean()}


class IssubclassFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'issubclass'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Boolean()}


# returns Iterator
class IterFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'iter'

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return ['__iter__']


class LenFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'len'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Integer()}


class ListFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'list'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {built_in_types.List()}


class LocalsFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'locals'


class MapFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'map'


class MaxFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'max'


class MemoryViewFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'memoryview'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {MemoryView()}


class MinFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'min'


class NextFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'next'

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return ['__next__']


class ObjectFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'object'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Object()}


class OctFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'oct'


class OpenFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'open'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {File()}


class OrdFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'ord'


class PowFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'pow'


class PrintFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'print'


class PropertyFunction(FunctionDecorator):
    @staticmethod
    def get_name() -> str:
        return 'property'


class RangeFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'range'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {RangeType()}


class ReprFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'repr'

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return ['__repr__']


# returns iterator
class ReversedFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'reversed'

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return ['__reversed__']


class RoundFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'round'


class SetFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'set'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {built_in_types.Set()}


class SetattrFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'setattr'


class SliceFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'slice'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Slice()}


class SortedFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'sorted'


class StaticmethodFunction(FunctionDecorator):
    @staticmethod
    def get_name() -> str:
        return 'staticmethod'


class StrFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'str'

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return ['__str__', '__repr__']

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {String()}


class SumFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'sum'


class SuperFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'super'


class TupleFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'tuple'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Tuple()}


class TypeFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'type'

    @staticmethod
    def get_type() -> Optional[Set[Union[DeclarationType]]]:
        return {Type()}


class VarsFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'vars'

    @staticmethod
    def get_override() -> Optional[List[str]]:
        return ['__dict__']


class ZipFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return 'zip'


class ImportFunction(BuiltInFunction):
    @staticmethod
    def get_name() -> str:
        return '__import__'


built_in_functions = inspect.getmembers(sys.modules[__name__],
                                        lambda x: inspect.isclass(x) and issubclass(x, BuiltInFunction) and
                                                  x is not BuiltInFunction and x is not FunctionDecorator)


def get_built_in_function(name: str) -> Optional[BuiltInFunction]:
    bif = [x for x in built_in_functions if x[1].get_name() == name]
    assert len(bif) <= 1
    if len(bif) == 0:
        return None

    return bif[0][1]()


def get_all_built_in_functions() -> List[BuiltInFunction]:
    return [bif[1]() for bif in built_in_functions]
