from typing import TypeVar, List, Iterable

T = TypeVar('T')


class UniqueList(List[T]):
    def __init__(self, seq=()):
        super().__init__()
        self.extend(seq)

    def append(self, obj: T) -> None:
        if obj in self:
            self.remove(obj)
        super().append(obj)

    def extend(self, iterable: Iterable[T]) -> None:
        for i in iterable:
            self.append(i)

    def insert(self, index: int, obj: T) -> None:
        if obj in self:
            self.remove(obj)
        super().insert(index, obj)
