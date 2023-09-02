from enum import Enum, unique, auto
from pathlib import PurePath, Path
from typing import Optional

from cc_python_parser.common.utils import process_file_content
from cc_python_parser.persistence.file_content_dto import FileContentDTO
from cc_python_parser.persistence.file_dto import FileDTO
from cc_python_parser.preprocessed_file import PreprocessedFile
from cc_python_parser.symbol_collector import SymbolCollector
from cc_python_parser.persistence.persistence import ModelPersistence


@unique
class ProcessStatus(Enum):
    WAITING = auto()
    PREPROCESSED = auto()
    PROCESSED = auto()


# TODO: expr_1; import; expr_2; - correct script -> not a problem
class FileInfo:
    def __init__(self, path: PurePath, persistence: ModelPersistence):
        self.path: PurePath = path
        self.symbol_collector: Optional[SymbolCollector] = None
        self.preprocessed_file: PreprocessedFile = PreprocessedFile(path)
        self.status: ProcessStatus = ProcessStatus.WAITING
        self.persistence: ModelPersistence = persistence

    def get_file(self):
        return self.path.name

    def get_file_name(self):
        return self.path.stem

    def preprocess_file(self, tree) -> None:
        self.preprocessed_file.visit(tree)
        self.status = ProcessStatus.PREPROCESSED

    def set_variable_collector(self, variable_collector: SymbolCollector) -> None:
        self.symbol_collector = variable_collector
        self.status = ProcessStatus.PROCESSED

    def create_dto(self) -> FileDTO:
        file_dto = FileDTO()
        file_dto.path = str(self.path)
        file_dto.file_name = self.path.name
        file_dto.timestamp = Path(self.path).stat().st_mtime
        file_dto.content = FileContentDTO(self.get_content(self.path))
        file_dto.parent = str(self.path.parent)
        file_dto.parse_status = self.get_parse_status(self.status)
        file_dto.documentation = self.preprocessed_file.documentation
        return file_dto

    def get_content(self, file: PurePath) -> str:
        content = ""

        def handle_file_content(c, _):
            nonlocal content
            content = c

        process_file_content(file, handle_file_content, self.persistence)
        return content

    @staticmethod
    def get_parse_status(status: ProcessStatus) -> int:
        if status == ProcessStatus.WAITING:
            return 0
        elif status == ProcessStatus.PREPROCESSED:
            return 1
        elif status == ProcessStatus.PROCESSED:
            return 2
        assert False
