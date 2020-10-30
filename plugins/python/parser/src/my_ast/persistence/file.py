from enum import unique, Enum
from pathlib import PurePath, Path
from my_ast.file_info import ProcessStatus, FileInfo
from my_ast.persistence.file_content import FileContent


@unique
class ParseStatus(Enum):
    PSNONE = 0
    PSPARTIALLYPARSED = 1
    PSFULLYPARSED = 2


class File:
    def __init__(self, file_info: FileInfo):
        # id?
        self.type: str = 'Python'   # dir, unknown, binary?
        self.path: PurePath = file_info.path
        self.file_name: str = file_info.file
        self.timestamp = Path(file_info.path).stat().st_mtime   # st_birthtime
        self.content: FileContent = FileContent(self.get_content(file_info.path))
        self.parent: PurePath = file_info.path.parent
        self.parse_status: ParseStatus = self.get_parse_status(file_info.status)

    @staticmethod
    def get_content(file: PurePath) -> str:
        with open(file) as f:
            return f.read()

    @staticmethod
    def get_parse_status(status: ProcessStatus) -> ParseStatus:
        if status == ProcessStatus.WAITING:
            return ParseStatus.PSNONE
        elif status == ProcessStatus.PREPROCESSED:
            return ParseStatus.PSPARTIALLYPARSED
        elif status == ProcessStatus.PROCESSED:
            return ParseStatus.PSFULLYPARSED
        assert False
