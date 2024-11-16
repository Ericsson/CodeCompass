from dataclasses import dataclass
from typing import List
from jedi import Project

@dataclass
class ParserConfig:
    root_path: str
    sys_path: List[str]
    venv_path: str | None = None
    project: Project | None = None 
    debug: bool = False
    stack_trace: bool = False
    safe_env: bool = False
    type_hint: bool = False
    file_refs: bool = True
    submodule_discovery: bool = True
    ast: bool = True
    ast_function_call: bool = True
    ast_import: bool = True
    ast_annotations: bool = True
    ast_inheritance: bool = True
    ast_function_signature: bool = True
