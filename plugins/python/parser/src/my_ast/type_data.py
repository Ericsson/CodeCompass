from abc import ABC, abstractmethod


class DeclarationType(ABC):
    def __init__(self, name: str, qualified_name: str):
        self.name = name
        self.qualified_name = qualified_name

    @abstractmethod
    def get_type_repr(self) -> str:
        pass

    @abstractmethod
    def __hash__(self):
        pass

    @abstractmethod
    def __eq__(self, other):
        pass

    @abstractmethod
    def __ne__(self, other):
        pass


class PlaceholderType(DeclarationType):
    def __init__(self, name: str):
        DeclarationType.__init__(self, name, "")

    def get_type_repr(self) -> str:
        return 'Placeholder'

    def __hash__(self):
        return hash(self.name)

    def __eq__(self, other):
        return isinstance(other, type(self)) and self.name == other.name

    def __ne__(self, other):
        return not self.__eq__(other)


class VariablePlaceholderType(PlaceholderType):
    pass


class InitVariablePlaceholderType(VariablePlaceholderType):
    pass


class FunctionPlaceholderType(PlaceholderType):
    pass
