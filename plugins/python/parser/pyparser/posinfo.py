from dataclasses import dataclass

@dataclass
class PosInfo:
    line_start: int = 0
    line_end: int = 0
    column_start: int = 0
    column_end: int = 0
    value: str = ""

