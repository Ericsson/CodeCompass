import typing
from typing import Optional, Any

from my_ast.base_data import TypeDeclaration
from my_ast.common.file_position import FilePosition


class BuiltIn(TypeDeclaration):
    def __init__(self, name: str):
        super().__init__(name, "builtins." + name, FilePosition.get_empty_file_position())

    def get_type_repr(self) -> str:
        return self.name

    def __hash__(self):
        return hash(self.name)

    def __eq__(self, other):
        return isinstance(other, type(self)) and self.name == other.name

    def __ne__(self, other):
        return not self.__eq__(other)


class GenericType:
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        if types is None:
            self.types = set()
        else:
            self.types = types

    def add_type(self, new_type):
        self.types.add(new_type)


class GenericBuiltInType(BuiltIn, GenericType):
    def __init__(self, name: str, types: Optional[typing.Set[Any]] = None):
        BuiltIn.__init__(self, name)
        GenericType.__init__(self, types)

    def get_type_repr(self) -> str:
        return self.name + '<' + ','.join({x.get_type_repr() for x in self.types}) + '>'


class Boolean(BuiltIn):
    def __init__(self):
        super().__init__('bool')


class Integer(BuiltIn):
    def __init__(self):
        super().__init__('int')


class Float(BuiltIn):
    def __init__(self):
        super().__init__('float')


class Complex(BuiltIn):
    def __init__(self):
        super().__init__('complex')


class String(BuiltIn):
    def __init__(self):
        super().__init__('str')


class List(GenericBuiltInType):
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        super().__init__('list', types)


class Tuple(GenericBuiltInType):
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        super().__init__('tuple', types)


class Range(BuiltIn):
    def __init__(self):
        super().__init__('range')


class Bytes(BuiltIn):
    def __init__(self):
        super().__init__('bytes')


class ByteArray(BuiltIn):
    def __init__(self):
        super().__init__('bytearray')


class MemoryView(BuiltIn):
    def __init__(self):
        super().__init__('memoryview')


class Set(GenericBuiltInType):
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        super().__init__('set', types)


class FrozenSet(GenericBuiltInType):
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        super().__init__('frozenset', types)


class Dictionary(GenericBuiltInType):
    def __init__(self, types: Optional[typing.Set[Any]] = None):
        super().__init__('dict', types)


class RangeType(BuiltIn):   # generic? (implement __index__ method)
    def __init__(self):
        super().__init__('range')


class NoneType(BuiltIn):
    def __init__(self):
        super().__init__('None')


class EllipsisType(BuiltIn):
    def __init__(self):
        super().__init__('Ellipsis')


class Slice(BuiltIn):
    def __init__(self):
        super().__init__('slice')


class Module(BuiltIn):
    def __init__(self):
        super().__init__('module')


class Function(BuiltIn):
    def __init__(self):
        super().__init__('function')


class Method(BuiltIn):
    def __init__(self):
        super().__init__('method')


class Lambda(BuiltIn):
    def __init__(self):
        super().__init__('lambda')


class Generator(BuiltIn):
    def __init__(self):
        super().__init__('generator')


class Type(BuiltIn):
    def __init__(self):
        super().__init__('type')


class Object(BuiltIn):
    def __init__(self):
        super().__init__('object')


class File(BuiltIn):
    def __init__(self):
        super().__init__('file')

# others: contextmanager, code (~str?), NotImplemented, internal
