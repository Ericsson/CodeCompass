from jedi.api.classes import Name
from typing import List
from hashlib import sha1
from parserutil import fnvHash
from parserlog import log, bcolors
from asthelper import ASTHelper
from posinfo import PosInfo

class PYName:
    id: int
    name: Name
    refid: int
    defs: List[Name]
    asthelper: ASTHelper | None

    def __init__(self, name: Name):
        self.name = name
        self.hashName = self.__getHashName()
        self.refid = self.hashName
        self.defs = []
        self.asthelper = None

    def addDefs(self, defs: List[Name], result):
        self.defs = defs

        if len(defs) > 0:
            self.refid = min(list(map(lambda e : PYName(e).hashName, defs)))
        else:
            self.__reportMissingDefinition(result)

        return self

    def addASTHelper(self, asthelper: ASTHelper):
        self.asthelper = asthelper
        return self

    def getNamePosInfo(self) -> PosInfo:
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

        return pos
    
    def getNodeInfo(self):
        node = {}
        node["id"] = self.hashName
        node["ref_id"] = self.refid
        node["module_name"] = self.name.module_name
        node["module_path"] = self.name.module_path
        node["full_name"] = self.name.full_name if self.name.full_name else ""

        pos = self.getNamePosInfo()
        node["line_start"] = pos.line_start
        node["line_end"] = pos.line_end
        node["column_start"] = pos.column_start + 1
        node["column_end"] = pos.column_end + 1
        node["value"] = pos.value

        node["type"] = self.name.type
        node["is_definition"] = self.name.is_definition()
        node["file_id"] = self.__getFileId()
        node["type_hint"] = self.__getNameTypeHint()
        node["is_builtin"] = self.name.in_builtin_module() or any(list(map(lambda x : x.in_builtin_module(), self.defs)))
        node["is_import"] = "import" in node["value"]

        parent = self.name.parent()
        node["parent"] = PYName(parent).hashName if parent else node["id"]

        node["is_call"] = False

        if self.asthelper:
            # Add func call
            node["is_call"] = self.asthelper.isFunctionCall(pos)

        return node

    def __getHashName(self) -> int:
        pos = self.getNamePosInfo() 
        s = f"{self.name.module_path}|{pos.line_start}|{pos.line_end}|{pos.column_start}|{pos.column_end}".encode("utf-8")
        hash = int(sha1(s).hexdigest(), 16) & 0xffffffffffffffff
        return hash

    def __getFileId(self):
        return fnvHash(str(self.name.module_path))
    
    def __reportMissingDefinition(self, result):
        pos = self.getNamePosInfo()
        if not self.name.is_definition() and self.name.type == 'module':
            log(f"{bcolors.FAIL}Missing {self.name.description} (file = {self.name.module_path} line = {pos.line_start})")
            result["status"] = "partial"

    def __getNameTypeHint(self):
        hint = ""
        try:
            res = self.name.get_type_hint()
            hint = res if res else ""
        except:
            pass

        return hint
