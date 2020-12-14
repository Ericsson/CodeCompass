import sys
import typing
import inspect
from abc import ABC, abstractmethod
from typing import Optional, Any

from cc_python_parser.base_data import TypeDeclaration
from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.persistence.class_dto import ClassDeclarationDTO


class BuiltIn(TypeDeclaration, ABC):
    def __init__(self):
        super().__init__(self.get_name(), "builtins.type." + self.get_name(), FilePosition.get_empty_file_position())

    def get_type_repr(self) -> str:
        return self.name

    def __hash__(self):
        return hash(self.name)

    def __eq__(self, other):
        return isinstance(other, type(self)) and self.name == other.name

    def __ne__(self, other):
        return not self.__eq__(other)

    def create_dto(self) -> ClassDeclarationDTO:
        return ClassDeclarationDTO(self.name, self.qualified_name, self.file_position,
                                   set(), [], set(), ClassDeclarationDTO.ClassMembersDTO(), "")

    @staticmethod
    @abstractmethod
    def get_name() -> str:
        pass


class GenericType:
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        if types is None:
            self.types = set()
        else:
            self.types = types

    def add_type(self, new_type):
        self.types.add(new_type)


class GenericBuiltInType(BuiltIn, GenericType, ABC):
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        BuiltIn.__init__(self)
        GenericType.__init__(self, types)

    def get_type_repr(self) -> str:
        return self.name + '<' + ','.join({x.get_type_repr() for x in self.types}) + '>'


class Boolean(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'bool'


class Integer(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'int'


class Float(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'float'


class Complex(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'complex'


class String(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'str'


class List(GenericBuiltInType):
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        super().__init__(types)

    @staticmethod
    def get_name() -> str:
        return 'list'


class Tuple(GenericBuiltInType):
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        super().__init__(types)

    @staticmethod
    def get_name() -> str:
        return 'tuple'


class Bytes(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'bytes'


class ByteArray(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'bytearray'


class MemoryView(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'memoryview'


class Set(GenericBuiltInType):
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        super().__init__(types)

    @staticmethod
    def get_name() -> str:
        return 'set'


class FrozenSet(GenericBuiltInType):
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        super().__init__(types)

    @staticmethod
    def get_name() -> str:
        return 'frozenset'


class Dictionary(GenericBuiltInType):
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        super().__init__(types)

    @staticmethod
    def get_name() -> str:
        return 'dict'


class RangeType(BuiltIn):  # generic? (implement __index__ method)
    @staticmethod
    def get_name() -> str:
        return 'range'


class NoneType(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'None'


class EllipsisType(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'Ellipsis'


class NotImplementedType(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'NotImplemented'


class Slice(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'slice'


class Module(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'module'


class Function(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'function'


class Method(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'method'


class Lambda(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'lambda'


class Generator(GenericBuiltInType):
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        super().__init__(types)

    @staticmethod
    def get_name() -> str:
        return 'generator'


class Type(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'type'


class Object(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'object'


class File(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'file'


class AnyType(BuiltIn):
    @staticmethod
    def get_name() -> str:
        return 'Any'


# others: contextmanager, code (~str?), internal

built_in_types = inspect.getmembers(sys.modules[__name__],
                                    lambda x: inspect.isclass(x) and issubclass(x, BuiltIn) and x is not BuiltIn and
                                              x is not GenericType and x is not GenericBuiltInType)


def get_built_in_type(name: str) -> Optional[BuiltIn]:
    bit = [x for x in built_in_types if x[1].get_name() == name]
    assert len(bit) <= 1
    if len(bit) == 0:
        return None

    return bit[0][1]()


def get_all_built_in_types() -> typing.List[BuiltIn]:
    return [bit[1]() for bit in built_in_types]
