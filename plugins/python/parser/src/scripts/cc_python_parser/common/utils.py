import ast
from pathlib import PurePath, Path
from typing import Final, List, Callable

from cc_python_parser.common.position import Range, Position


ENCODINGS: Final[List[str]] = ['utf-8', 'iso-8859-1']


def process_file_content(path: PurePath, func: Callable[[str, int], None]) -> None:
    if not Path(path).exists():
        return
    for encoding in ENCODINGS:
        try:
            with open(str(path), "r", encoding=encoding) as source:
                content = source.read()
                source.seek(0)
                func(content, len(source.readlines()))
        except UnicodeDecodeError:
            continue
        except FileNotFoundError:
            print(f"File not found: {path}")
            continue
        else:
            return
    print(f"Unhandled encoding in {str(path)}")


def has_attr(obj, attrs) -> bool:
    for attr in attrs.split("."):
        if hasattr(obj, attr):
            obj = getattr(obj, attr)
        else:
            return False
    return True


def create_range_from_ast_node(node: ast.AST) -> Range:
    start_pos = Position(node.lineno, node.col_offset)
    end_pos = Position(node.end_lineno, node.end_col_offset)
    return Range(start_pos, end_pos)
