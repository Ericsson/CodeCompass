from dataclasses import dataclass

@dataclass
class ParseResult:
    path: str
    status: str = "full"
    nodes = []
    imports = []
