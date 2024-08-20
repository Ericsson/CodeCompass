import ast
from typing import List
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

    def getFunctionCalls(self) -> List[ast.AST]:
        return list(filter(lambda e : isinstance(e, ast.Call), self.astNodes))

    def isFunctionCall(self, pos: PosInfo):
        for e in self.getFunctionCalls():
            if (not isinstance(e, ast.Call) or not isinstance(e.func, ast.Name)):
               continue
            
            func = e.func
            if (func.lineno == pos.line_start and
                func.end_lineno == pos.line_end and
                func.col_offset == pos.column_start and
                func.end_col_offset == pos.column_end):
                return True

        return False
 
