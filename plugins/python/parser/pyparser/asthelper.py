import ast
from typing import cast, List
from posinfo import PosInfo
from nodeinfo import NodeInfo
from parserutil import fnvHash, getHashName

class ASTHelper:
    astNodes: List[ast.AST]
    calls: List[ast.Call]
    imports: List[ast.Import | ast.ImportFrom]
    functions: List[ast.FunctionDef]
    path: str
    source: str
    lines: List[str]

    def __init__(self, path, source):
        self.astNodes = []
        self.calls = []
        self.imports = []
        self.functions = []
        self.path = path
        self.file_id = fnvHash(self.path)
        self.source = source
        self.lines = source.split("\n")
        
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

    def getAnnotations(self):
        results = []

        for func in self.functions:
            if not (isinstance(func.returns, ast.Subscript) and
                func.lineno and
                func.end_lineno and
                func.col_offset and
                func.end_col_offset):
                continue

            sub = func.returns
            line_start = sub.lineno - 1
            line_end = sub.end_lineno - 1 if sub.end_lineno else line_start
            col_start = sub.col_offset
            col_end = sub.end_col_offset if sub.end_col_offset else col_start

            if line_start == line_end:
                return_type = self.lines[line_start][col_start:col_end]
            else:
                return_type = self.lines[line_start][col_start:]
                for l in range(line_start + 1, line_end):
                    return_type += self.lines[l]
                return_type += self.lines[line_end][:col_end]

            if return_type:
                funcpos = PosInfo(line_start=func.lineno, line_end=func.end_lineno, column_start=func.col_offset, column_end=func.end_col_offset)
                subpos = PosInfo(line_start=sub.lineno, line_end=line_end + 1, column_start=sub.col_offset, column_end=col_end)

                funchash = getHashName(self.path, funcpos)
                subhash = getHashName(self.path, subpos)

                nodeinfo = NodeInfo()
                nodeinfo.id = subhash
                nodeinfo.ref_id = subhash
                nodeinfo.parent = funchash
                nodeinfo.parent_function = funchash
                nodeinfo.line_start = subpos.line_start
                nodeinfo.line_end = subpos.line_end
                nodeinfo.column_start = subpos.column_start
                nodeinfo.column_end = subpos.column_end
                nodeinfo.file_id = self.file_id
                nodeinfo.type = "annotation"
                nodeinfo.value = return_type

                results.append(nodeinfo)

        return results
