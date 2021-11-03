from cc_python_parser.common.position import Range


class FilePositionDTO:
    def __init__(self, file: str, r: Range):
        self.file: str = file
        self.range: Range = r

    def __hash__(self):
        return hash(str(self.file)) ^ hash(self.range)

    def __eq__(self, other):
        return self.file == other.file and self.range == other.range

    def __ne__(self, other):
        return not self.__eq__(other)
