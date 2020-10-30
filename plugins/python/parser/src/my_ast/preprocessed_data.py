from my_ast.base_data import Declaration
from my_ast.common.file_position import FilePosition


class PreprocessedDeclaration(Declaration):
    def __init__(self, name: str):
        file_position = FilePosition.get_empty_file_position()
        Declaration.__init__(self, name, "", file_position)     # TODO: need qualified name?
