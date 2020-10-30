from typing import List

import my_ast.persistence.build_source_target as bst


class BuildAction:
    def __init__(self, command: str, sources: List[bst.BuildSource], targets: List[bst.BuildTarget]):
        self.command = command
        self.type = 0   # compile
        self.build_sources: List[bst.BuildSource] = sources
        self.build_target: List[bst.BuildTarget] = targets
