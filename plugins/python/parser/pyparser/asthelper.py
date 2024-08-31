import ast
from typing import cast, List
from posinfo import PosInfo

class ASTHelper:
    astNodes: List[ast.AST]
    source: str

    def __init__(self, source):
        self.astNodes = []
        self.source = source
        
        try:
            tree = ast.parse(source)
            self.astNodes = list(ast.walk(tree))
        except:
            pass

    def getFunctionCalls(self) -> List[ast.Call]:
        return cast(List[ast.Call], list(filter(lambda e : isinstance(e, ast.Call), self.astNodes)))

    def isFunctionCall(self, pos: PosInfo):
        for e in self.getFunctionCalls():
            func = e.func

            if (isinstance(func, ast.Name)):
                if (func.lineno == pos.line_start and
                    func.end_lineno == pos.line_end and
                    func.col_offset == pos.column_start and
                    func.end_col_offset == pos.column_end):
                    return True

            elif (isinstance(func, ast.Attribute)):
                if func.end_col_offset:
                    col_start = func.end_col_offset - len(func.attr)
                else:
                    col_start = 0

                if (func.end_lineno == pos.line_start and
                    func.end_lineno == pos.line_end and
                    col_start == pos.column_start and
                    func.end_col_offset == pos.column_end):
                    return True

        return False
 
