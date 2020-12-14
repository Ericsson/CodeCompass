from cc_python_parser.base_data import Declaration
from cc_python_parser.common.file_position import FilePosition


class PreprocessedDeclaration(Declaration):
    def __init__(self, name: str, file_position: FilePosition):
        Declaration.__init__(self, name, "", file_position)     # TODO: need qualified name?
