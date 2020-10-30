from my_ast.common.position import Position


class Location:
    def __init__(self, path: str, file: str, pos: Position):
        self.path = path
        self.file = file
        self.position = pos
