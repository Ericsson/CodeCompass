from collections import Counter
from typing import List, TypeVar, Generic

T = TypeVar('T')


class HashableList(Generic[T], List[T]):
    def __hash__(self):
        return hash(e for e in self)

    def __eq__(self, other):
        return isinstance(other, type(self)) and Counter(self) == Counter(other)

    def __ne__(self, other):
        return not self.__eq__(other)


class OrderedHashableList(Generic[T], List[T]):
    def __hash__(self):
        return hash(e for e in self)

    def __eq__(self, other):
        if not isinstance(other, type(self)) or len(other) != len(self):
            return False
        for i in range(0, len(self)):
            if self[i] != other[i]:
                return False
        return True

    def __ne__(self, other):
        return not self.__eq__(other)
