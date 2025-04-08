from typing import List
from jedi import Script
from jedi.api.classes import Name
from pyname import PYName
from parserconfig import ParserConfig
from parserlog import log, bcolors
import traceback

class PYReference:
    script: Script
    names: List[Name]
    config: ParserConfig

    def __init__(self, config: ParserConfig, script: Script, names: List[Name]):
        self.config = config
        self.script = script
        self.names = names
        self.refmap = {}

        if self.config.file_refs:
            self.__lookupFileRefs()

    def getDefs(self, name: Name) -> List[PYName]:
        try:
            defs = name.goto(follow_imports = True, follow_builtin_imports = True)
            defs = list(map(lambda e : PYName(e), defs))
            return defs
        except:
            if self.config.debug:
                log(f"{bcolors.FAIL}Failed to find definition! (file = {str(name.module_path)} line = {name.line} column = {name.column})")
                if self.config.stack_trace:
                    traceback.print_exc()

            return []

    def getFileRefs(self, name: Name) -> List[PYName]:
        id = PYName(name).hashName

        if id in self.refmap:
            return self.refmap[id]
        else:
            return []

    def __lookupFileRefs(self):
        for x in self.names:
            if not(x.is_definition()):
                continue

            try:
                refs = self.script.get_references(x.line, x.column, scope = "file")
                refs = list(map(lambda e : PYName(e), refs))
                refs.append(PYName(x))

                for r in refs:
                    self.refmap[r.hashName] = refs
            except:
                if self.config.debug:
                    log(f"{bcolors.FAIL}Failed to find references! (file = {str(x.module_path)} line = {x.line} column = {x.column})")
                    if self.config.stack_trace:
                        traceback.print_exc()

