from enum import unique, Enum

from cc_python_parser.persistence.documented_dto import DocumentedDTO
from cc_python_parser.persistence.file_content_dto import FileContentDTO


@unique
class ParseStatus(Enum):
    PSNONE = 0
    PSPARTIALLYPARSED = 1
    PSFULLYPARSED = 2


class FileDTO(DocumentedDTO):
    def __init__(self):
        super().__init__("")
        self.type: str = 'Python'   # dir, unknown, binary?
        self.path: str = ""
        self.file_name: str = ""
        self.timestamp = ""
        self.content: FileContentDTO = FileContentDTO("")
        self.parent: str = ""
        self.parse_status: int = 0  # PSNONE
