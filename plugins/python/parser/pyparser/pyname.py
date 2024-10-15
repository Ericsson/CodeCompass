from jedi.api.classes import Name
from typing import List
from parserutil import fnvHash, getHashName
from parserlog import log, bcolors
from asthelper import ASTHelper
from posinfo import PosInfo
from pybuiltin import PYBuiltin
from parserconfig import ParserConfig
from nodeinfo import NodeInfo

class PYName:
    id: int
    path: str | None
    name: Name
    pos: PosInfo
    refid: int
    defs: List['PYName']
    asthelper: ASTHelper | None
    config: ParserConfig | None

    def __init__(self, name: Name):
        self.name = name
        self.path = str(self.name.module_path) if self.name.module_path else None
        self.pos = self.__getNamePosInfo()
        self.hashName = getHashName(self.path, self.pos)
        self.refid = self.hashName
        self.defs = []
        self.asthelper = None
        self.config = None

    def addDefs(self, defs: List['PYName'], result):
        self.defs = defs

        if len(defs) > 0:
            self.refid = min(list(map(lambda e : e.hashName, defs)))
        else:
            self.__reportMissingDefinition(result)

        return self

    def addRefs(self, refs: List['PYName']):
        defs = list(filter(lambda e : e.name.is_definition(), refs))

        if len(defs) > 0:
            self.refid = min(list(map(lambda e : e.hashName, defs)))

        return self

    def addASTHelper(self, asthelper: ASTHelper):
        self.asthelper = asthelper
        return self

    def addConfig(self, config: ParserConfig):
        self.config = config
        return self

    def __getNamePosInfo(self) -> PosInfo:
        pos = PosInfo()

        start_pos = self.name.get_definition_start_position()
        end_pos = self.name.get_definition_end_position()

        if start_pos and end_pos:
            pos.line_start = start_pos[0]
            pos.line_end = end_pos[0]
            pos.column_start = start_pos[1]
            pos.column_end = end_pos[1]

            if pos.line_start == pos.line_end:
                pos.value = self.name.get_line_code()[pos.column_start:pos.column_end]
            else:
                pos.value = self.name.get_line_code()[pos.column_start:]

        if (self.path and
            pos.line_start == 0 and pos.line_end == 0 and
            pos.column_start == 0 and pos.column_end == 0):
            pos.value = self.path.split("/")[-1]

        return pos
    
    def getNodeInfo(self) -> NodeInfo:
        node = NodeInfo()
        node.id = self.hashName
        node.ref_id = self.refid
        node.module_name = self.name.module_name
        node.full_name = self.name.full_name if self.name.full_name else ""

        node.line_start = self.pos.line_start
        node.line_end = self.pos.line_end
        node.column_start = self.pos.column_start + 1
        node.column_end = self.pos.column_end + 1
        node.value = self.pos.value

        node.type = self.name.type
        node.is_definition = self.name.is_definition()
        node.file_id = self.__getFileId()
        node.type_hint = self.__getNameTypeHint()
        node.is_builtin = PYBuiltin.isBuiltin(self.name) or any(list(map(lambda x : PYBuiltin.isBuiltin(x.name), self.defs)))

        parent = self.name.parent()
        node.parent = PYName(parent).hashName if parent else node.id
        node.parent_function = self.__getParentFunction()

        if self.asthelper:
            node.is_call = self.asthelper.isFunctionCall(self.pos)
            node.is_import = self.asthelper.isImport(self.pos)

            if node.type == "param":
                node.type = "astparam" if self.asthelper.isFunctionParam(self.pos) else "param"

            subclass = self.asthelper.getSubclass(self.pos)
            if subclass:
                node.type = "baseclass"
                node.parent = subclass

        return node

    def __getFileId(self):
        if self.path:
            return fnvHash(self.path)
        else:
            return 0
    
    def __reportMissingDefinition(self, result):
        if not self.name.is_definition() and self.name.type == 'module':
            log(f"{bcolors.FAIL}Missing {self.name.description} (file = {self.path} line = {self.pos.line_start})")
            result["status"] = "partial"

    def __getParentFunction(self):
        try:
            node = self.name
            for _ in range(0,10):
                parent: Name | None = node.parent()
                if parent and parent.type == "function" and parent.is_definition():
                    return PYName(parent).hashName
                elif parent:
                    node = parent
                else:
                    break
        except:
            pass

        return self.hashName

    def __getNameTypeHint(self):
        hint = ""
        if not (self.config and self.config.type_hint):
            return hint

        try:
            res = self.name.get_type_hint()
            hint = res if res else ""
        except:
            pass

        return hint
