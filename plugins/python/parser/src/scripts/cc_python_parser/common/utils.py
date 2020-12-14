import ast
from typing import Final, List

from cc_python_parser.common.position import Range, Position


ENCODINGS: Final[List[str]] = ['utf-8', 'iso-8859-1']


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
