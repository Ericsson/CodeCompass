import ast
from pathlib import PurePath
from typing import List, Dict, Optional, Union

from cc_python_parser.base_data import Declaration
from cc_python_parser.common.file_position import FilePosition
from cc_python_parser.common.utils import has_attr, create_range_from_ast_node
from cc_python_parser.preprocessed_data import PreprocessedDeclaration
from cc_python_parser.preprocessed_function import PreprocessedFunction
from cc_python_parser.preprocessed_variable import PreprocessedVariable


class PreprocessedClass(PreprocessedDeclaration):
    def __init__(self, name: str, file_position: FilePosition):
        super().__init__(name, file_position)
        self.methods: List[PreprocessedFunction] = []
        self.attributes: List[PreprocessedVariable] = []
        self.classes: List[PreprocessedClass] = []
        self.type_usages: List[Declaration] = []

    def append_method(self, name: str, file_position: FilePosition, node: Union[ast.FunctionDef, ast.AsyncFunctionDef]):
        self.methods.append(PreprocessedFunction(name, file_position, node))

    def append_attribute(self, name: str, file_position: FilePosition):
        self.attributes.append(PreprocessedVariable(name, file_position))

    def append_class(self, nested_class: 'PreprocessedClass'):  # 'type' -> in case of type(self)
        self.classes.append(nested_class)


class PreprocessedClassCollector(ast.NodeVisitor):
    def __init__(self, path: Optional[PurePath]):
        self.path = path
        self.classes: List[PreprocessedClass] = []
        self.class_list: List[int] = []
        self.class_nest_class_map: Dict[PreprocessedClass, List[ast.ClassDef]] = {}

    def append_class(self, node: ast.ClassDef):
        self.class_list.append(len(self.classes))
        preprocessed_class = PreprocessedClass(node.name, FilePosition(self.path, create_range_from_ast_node(node)))
        self.classes.append(preprocessed_class)
        self.handle_nested_class(node, preprocessed_class)
        for member in node.body:
            if isinstance(member, (ast.FunctionDef, ast.AsyncFunctionDef)):
                self.append_method(member, FilePosition(self.path, create_range_from_ast_node(member)))
            elif isinstance(member, ast.Assign):
                self.append_attribute(member, FilePosition(self.path, create_range_from_ast_node(member)))
            elif isinstance(member, ast.ClassDef):
                self.append_nested_class(preprocessed_class, member)
            elif isinstance(member, ast.Pass):
                pass
            elif isinstance(member, ast.Expr) and hasattr(member, 'value') and isinstance(member.value, ast.Constant) \
                    and hasattr(member.value, "value") and isinstance(member.value.value, str):
                pass  # documentation comment
            else:
                pass
                # assert False, "Unknown class member: " + str(type(member))

    def handle_nested_class(self, node: ast.ClassDef, preprocessed_class: PreprocessedClass):
        for parent_class in self.class_nest_class_map:
            if node in self.class_nest_class_map[parent_class]:
                parent_class.append_class(preprocessed_class)
                self.class_nest_class_map[parent_class].remove(node)

    def class_processed(self):
        del self.class_list[-1]

    def append_method(self, node: Union[ast.FunctionDef, ast.AsyncFunctionDef], file_position: FilePosition):
        self.classes[self.class_list[-1]].append_method(node.name, file_position, node)
        if node.name == '__init__':
            self.visit(node)

    def append_attribute(self, node: ast.Assign, file_position: FilePosition):
        for attribute in node.targets:
            if hasattr(attribute, 'id'):  # class variable
                self.classes[self.class_list[-1]].append_attribute(attribute.id, file_position)
            elif hasattr(attribute, 'attr') and has_attr(attribute, 'value.id') and attribute.value.id == 'self':
                self.classes[self.class_list[-1]].append_attribute(attribute.attr, file_position)

    def append_nested_class(self, node: PreprocessedClass, member: ast.ClassDef):
        if node in self.class_nest_class_map:
            self.class_nest_class_map[node].append(member)
        else:
            self.class_nest_class_map[node] = [member]

    def visit_Assign(self, node: ast.Assign):
        self.append_attribute(node, FilePosition(self.path, create_range_from_ast_node(node)))
        self.generic_visit(node)
