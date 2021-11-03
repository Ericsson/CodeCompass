from typing import List, Union, Set, Optional

import cc_python_parser.base_data as data
from cc_python_parser.built_in_types import BuiltIn
from cc_python_parser.class_data import ClassDeclaration
from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.function_data import FunctionDeclaration


class ClassInitDeclaration(FunctionDeclaration):
    def __init__(self, class_declaration: ClassDeclaration):
        super().__init__(class_declaration.name, class_declaration.qualified_name, class_declaration.file_position,
                         [], "")
        self.class_declaration = class_declaration
