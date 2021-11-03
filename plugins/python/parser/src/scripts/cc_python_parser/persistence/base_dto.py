from typing import List, Set

from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.persistence.file_position_dto import FilePositionDTO

PUBLIC_DECLARATION = "public"
SEMI_PRIVATE_DECLARATION = "semiprivate"
PRIVATE_DECLARATION = "private"


def create_file_position_dto(file_position: FilePosition) -> FilePositionDTO:
    return FilePositionDTO(str(file_position.file), file_position.range)


class UsageDTO:
    def __init__(self, file_position: FilePosition):
        self.file_position: FilePositionDTO = create_file_position_dto(file_position)


class DeclarationDTO:
    def __init__(self, name: str, qualified_name: str, file_position: FilePosition, types: Set[str],
                 usages: List[UsageDTO]):
        self.name: str = name
        self.qualified_name: str = qualified_name
        self.file_position: FilePositionDTO = create_file_position_dto(file_position)
        self.type: List[str] = list(types)  # qualified names
        self.usages: List[UsageDTO] = usages
        self.visibility: str = self.get_visibility()

    def get_visibility(self) -> str:
        if self.name.startswith('__'):
            return PRIVATE_DECLARATION
        elif self.name.startswith('_'):
            return SEMI_PRIVATE_DECLARATION
        else:
            return PUBLIC_DECLARATION
