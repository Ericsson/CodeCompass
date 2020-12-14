class Position:
    def __init__(self, line: int, col: int):
        self.line = line
        self.column = col

    def __str__(self):
        return "line: " + str(self.line) + ", column: " + str(self.column)

    def __repr__(self):
        return self.__str__()

    def __eq__(self, other):
        return self.line == other.line and self.column == other.column

    def __ne__(self, other):
        return not self.__eq__(other)

    def __hash__(self):
        return hash(self.line) ^ hash(self.column)

    @staticmethod
    def get_empty_position():
        return Position(0, 0)


class Range:
    def __init__(self, start_pos: Position, end_pos: Position):
        self.start_position = start_pos
        self.end_position = end_pos

    def __str__(self):
        return "Start position: " + str(self.start_position) + " - End position: " + str(self.end_position)

    def __repr__(self):
        return self.__str__()

    def __hash__(self):
        return hash(self.start_position) ^ hash(self.end_position)

    def __eq__(self, other):
        return self.start_position == other.start_position and self.end_position == other.end_position

    def __ne__(self, other):
        return not self.__eq__(other)

    @staticmethod
    def get_empty_range():
        return Range(Position.get_empty_position(), Position.get_empty_position())
