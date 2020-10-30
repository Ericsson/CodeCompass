from pathlib import PurePath
from typing import Optional

from my_ast.common.position import Range


class FilePosition:
    def __init__(self, file: Optional[PurePath], r: Range):
        self.file: Optional[PurePath] = file    # Optional -> builtins
        self.range: Range = r

    def __str__(self):
        return "File: " + str(self.file) + " - " + str(self.range)

    def __repr__(self):
        return self.__str__()

    @staticmethod
    def get_empty_file_position():
        return FilePosition(None, Range.get_empty_range())
