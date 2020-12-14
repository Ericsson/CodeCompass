from pathlib import PurePath
from typing import Optional

from cc_python_parser.common.position import Range


class FilePosition:
    def __init__(self, file: Optional[PurePath], r: Range):
        self.file: Optional[PurePath] = file    # Optional -> builtins
        self.range: Range = r

    def __str__(self):
        return "File: " + str(self.file) + " - " + str(self.range)

    def __repr__(self):
        return self.__str__()

    def __hash__(self):
        return hash(str(self.file)) ^ hash(self.range)

    def __eq__(self, other):
        return self.file == other.file and self.range == other.range

    def __ne__(self, other):
        return not self.__eq__(other)

    @staticmethod
    def get_empty_file_position():
        return FilePosition(None, Range.get_empty_range())
