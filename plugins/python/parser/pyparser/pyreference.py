from typing import List
from jedi import Script
from jedi.api.classes import Name
from pyname import PYName

class PYReference:
    script: Script
    names: List[Name]

    def __init__(self, config, script: Script, names: List[Name]):
        self.script = script
        self.names = names
        self.refmap = {}

        if config["file_refs"]:
            self.__lookupFileRefs()

    def getDefs(self, name: Name) -> List[PYName]:
        defs = name.goto(follow_imports = True, follow_builtin_imports = True)
        defs = list(map(lambda e : PYName(e), defs))
        return defs

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

            refs = self.script.get_references(x.line, x.column, scope = "file")
            refs = list(map(lambda e : PYName(e), refs))
            refs.append(PYName(x))

            for r in refs:
                self.refmap[r.hashName] = refs

