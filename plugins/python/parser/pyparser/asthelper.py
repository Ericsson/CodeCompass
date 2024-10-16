import ast
from typing import cast, List
from posinfo import PosInfo
from nodeinfo import NodeInfo
from parserutil import fnvHash, getHashName
from parserconfig import ParserConfig

class ASTHelper:
    astNodes: List[ast.AST]
    calls: List[ast.Call]
    imports: List[ast.Import | ast.ImportFrom]
    functions: List[ast.FunctionDef]
    classes: List[ast.ClassDef]
    path: str
    source: str
    lines: List[str]

    def __init__(self, path: str, source: str, config: ParserConfig):
        self.astNodes = []
        self.calls = []
        self.imports = []
        self.functions = []
        self.classes = []
        self.path = path
        self.file_id = fnvHash(self.path)
        self.source = source
        self.lines = source.split("\n")
        self.config = config
        
        try:
            tree = ast.parse(source)
            self.astNodes = list(ast.walk(tree))
        except:
            pass

        if config.ast:
            if self.config.ast_function_call:
                self.calls = self.__getFunctionCalls()

            if self.config.ast_import:
                self.imports = self.__getImports()

            self.functions = self.__getFunctions()
            self.classes = self.__getClasses()

    def __getFunctionCalls(self) -> List[ast.Call]:
        return cast(List[ast.Call], list(filter(lambda e : isinstance(e, ast.Call), self.astNodes)))

    def __getFunctions(self) -> List[ast.FunctionDef]:
        return cast(List[ast.FunctionDef], list(filter(lambda e : isinstance(e, ast.FunctionDef), self.astNodes)))

    def __getClasses(self) -> List[ast.ClassDef]:
        return cast(List[ast.ClassDef], list(filter(lambda e : isinstance(e, ast.ClassDef), self.astNodes)))

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
                if isinstance(func.end_col_offset, int):
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

    def getFunctionParam(self, pos: PosInfo) -> str | None:
        for func in self.functions:
            if isinstance(func.args, ast.arguments) and func.args.args:
                for e in func.args.args:
                    if (e.lineno == pos.line_start and
                        e.col_offset == pos.column_start):
                        return self.__getPosValue(pos)

        return None

    def getSubclass(self, pos: PosInfo) -> int | None:
        if not (self.config.ast_inheritance):
            return None

        for cls in self.classes:
            if not (isinstance(cls.lineno, int) and
                isinstance(cls.end_lineno, int) and
                isinstance(cls.col_offset, int)):
                continue

            for e in cls.bases:
                if (isinstance(e, ast.Name) and
                    e.lineno == pos.line_start and
                    e.end_lineno == pos.line_end and
                    e.col_offset == pos.column_start and
                    e.end_col_offset == pos.column_end):

                    col_end = len(self.lines[cls.end_lineno - 1])
                    subpos = PosInfo(line_start=cls.lineno, line_end=cls.end_lineno, column_start=cls.col_offset, column_end=col_end)
                    subhash = getHashName(self.path, subpos)
                    return subhash

        return None

    def __getPosValue(self, pos: PosInfo) -> str | None:
        if not (pos.line_start >= 1 and pos.line_end >= 1):
            return None

        if pos.line_start == pos.line_end:
            value = self.lines[pos.line_start - 1][pos.column_start:pos.column_end]
        else:
            value = self.lines[pos.line_start - 1][pos.column_start:]
            for l in range(pos.line_start, pos.line_end - 1):
                value += self.lines[l]
            value += self.lines[pos.line_end - 1][:pos.column_end]

        if value:
            return value
        else:
            return None

    def __getASTValue(self, node: ast.expr) -> PosInfo | None:
        line_end = node.end_lineno if node.end_lineno else node.lineno
        col_end = node.end_col_offset if node.end_col_offset else node.col_offset

        pos = PosInfo(line_start=node.lineno, line_end=line_end, column_start=node.col_offset, column_end=col_end)
        value = None

        if isinstance(node, ast.Subscript) or isinstance(node, ast.Attribute):
            value = self.__getPosValue(pos)
        elif isinstance(node, ast.Name):
            value = node.id
        elif isinstance(node, ast.Constant):
            value = str(node.value)

        if value:
            pos.value = value
            return pos
        else:
            return None

    def __getFunctionReturnAnnotation(self, func: ast.FunctionDef) -> PosInfo | None:
        if func.returns:
            return self.__getASTValue(func.returns)
        else:
            return None

    def __getArgumentAnnotation(self, arg: ast.arg) -> PosInfo | None:
        if arg.annotation:
            return self.__getASTValue(arg.annotation)
        else:
            return None

    def getAnnotations(self):
        if not (self.config.ast and self.config.ast_annotations):
            return []

        results = []

        for func in self.functions:
            if not (isinstance(func.lineno, int) and
                isinstance(func.end_lineno, int) and
                isinstance(func.col_offset, int) and
                isinstance(func.end_col_offset, int)):
                continue

            subpos = self.__getFunctionReturnAnnotation(func)

            if subpos:
                funcpos = PosInfo(line_start=func.lineno, line_end=func.end_lineno, column_start=func.col_offset, column_end=func.end_col_offset)
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
                nodeinfo.value = subpos.value

                results.append(nodeinfo)

        return results

    def getFunctionSignature(self, pos: PosInfo) -> str | None:
        if not (self.config.ast_function_signature):
            return None

        for func in self.functions:
            if not (isinstance(func.lineno, int) and
                isinstance(func.end_lineno, int) and
                isinstance(func.col_offset, int) and
                isinstance(func.end_col_offset, int)):
                continue

            if not (func.lineno == pos.line_start and
                func.end_lineno == pos.line_end and
                func.col_offset == pos.column_start and
                func.end_col_offset == pos.column_end):
                continue

            sign = "def " + func.name
            sign += "("

            first = True
            for arg in func.args.args:
                if first:
                    first = False
                else:
                    sign += ", "

                sign += arg.arg

                param_annotation: PosInfo | None = self.__getArgumentAnnotation(arg)
                if param_annotation:
                    sign += ": " + param_annotation.value

            sign += ")"

            return_annotation: PosInfo | None = self.__getFunctionReturnAnnotation(func)
            if return_annotation:
                sign += " -> " + return_annotation.value

            return sign

        return None
