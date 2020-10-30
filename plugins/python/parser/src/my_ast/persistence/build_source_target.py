import my_ast.persistence.build_action as ba
from my_ast.persistence.file_dto import FileDTO


class BuildSource:
    def __init__(self, file: FileDTO, action: ba.BuildAction):
        self.file: FileDTO = file
        self.action: ba.BuildAction = action


class BuildTarget:
    def __init__(self, file: FileDTO, action: ba.BuildAction):
        self.file: FileDTO = file
        self.action: ba.BuildAction = action
