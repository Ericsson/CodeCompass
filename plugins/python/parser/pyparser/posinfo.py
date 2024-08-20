class PosInfo:
    line_start: int
    line_end: int
    column_start: int
    column_end: int
    value: str

    def __init__(self):
        self.line_start = 0
        self.line_end = 0
        self.column_start = 0
        self.column_end = 0
        self.value = ""

