from pathlib import PurePath
from typing import Callable     # python 3.5 or above


class ParseException:
    def __init__(self, is_dictionary_exception: Callable[[PurePath], bool],
                 is_file_exception: Callable[[PurePath], bool]):
        self.is_dictionary_exception = is_dictionary_exception
        self.is_file_exception = is_file_exception

    def is_dictionary_exception(self, dictionary: PurePath) -> bool:
        return self.is_dictionary_exception(dictionary)

    def is_file_exception(self, file: PurePath) -> bool:
        return self.is_file_exception(file)


class DefaultParseException(ParseException):
    def __init__(self):
        super().__init__(lambda x: False, lambda x: False)
