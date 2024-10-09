import ast
from typing import cast, List
from posinfo import PosInfo

class ASTHelper:
    astNodes: List[ast.AST]
    calls: List[ast.Call]
    imports: List[ast.Import | ast.ImportFrom]
    functions: List[ast.FunctionDef]
    path: str
    source: str

    def __init__(self, path, source):
        self.astNodes = []
        self.calls = []
        self.imports = []
        self.functions = []
        self.path = path
        self.source = source
        
        try:
            tree = ast.parse(source)
            self.astNodes = list(ast.walk(tree))
            self.calls = self.__getFunctionCalls()
            self.imports = self.__getImports()
            self.functions = self.__getFunctions()
        except:
            pass

    def __getFunctionCalls(self) -> List[ast.Call]:
        return cast(List[ast.Call], list(filter(lambda e : isinstance(e, ast.Call), self.astNodes)))

    def __getFunctions(self) -> List[ast.FunctionDef]:
        return cast(List[ast.FunctionDef], list(filter(lambda e : isinstance(e, ast.FunctionDef), self.astNodes)))

    def __getImports(self) -> List[ast.Import | ast.ImportFrom]:
        return cast(List[ast.Import | ast.ImportFrom], list(filter(lambda e : isinstance(e, ast.Import) or isinstance(e, ast.ImportFrom), self.astNodes)))

    def isFunctionCall(self, pos: PosInfo):
        for e in self.calls:
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

    def isImport(self, pos: PosInfo):
        for e in self.imports:
            if (e.lineno == pos.line_start and
                e.end_lineno == pos.line_end and
                e.col_offset == pos.column_start and
                e.end_col_offset == pos.column_end):
                return True

        return False

    def isFunctionParam(self, pos: PosInfo):
        for func in self.functions:
            if isinstance(func.args, ast.arguments) and func.args.args:
                for e in func.args.args:
                    if (e.lineno == pos.line_start and
                        e.end_lineno == pos.line_end and
                        e.col_offset == pos.column_start):
                        return True

        return False
