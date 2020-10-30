from typing import List, Set

from my_ast.common.file_position import FilePosition


class UsageDTO:
    def __init__(self, file_position: FilePosition):
        self.file_position: FilePosition = file_position


class DeclarationDTO:
    def __init__(self, name: str, qualified_name: str, file_position: FilePosition, types: Set[str],
                 usages: List[UsageDTO]):
        self.name: str = name
        self.qualified_name: str = qualified_name
        self.file_position: FilePosition = file_position
        self.type: Set[str] = types      # qualified names
        self.usages: List[UsageDTO] = usages
