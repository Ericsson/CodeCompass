from dataclasses import dataclass, field

@dataclass
class ParseResult:
    path: str
    status: str = "full"
    nodes: list = field(default_factory=list)
    imports: list = field(default_factory=list)
