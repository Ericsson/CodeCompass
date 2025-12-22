from dataclasses import dataclass

@dataclass
class NodeInfo:
    id: int = 0
    ref_id: int = 0
    file_id: int = 0

    module_name: str = ""
    full_name: str = ""

    parent: int = 0
    parent_function: int = 0

    line_start: int = 1
    line_end: int = 1
    column_start: int = 1
    column_end: int = 1
    value: str = ""

    type: str = ""
    type_hint: str = ""
    is_definition: bool = False
    is_builtin: bool = False
    is_call: bool = False
    is_import: bool = False
