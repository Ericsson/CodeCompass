import ast
from typing import List, Dict

from my_ast.base_data import Declaration
from my_ast.common.utils import has_attr
from my_ast.preprocessed_data import PreprocessedDeclaration
from my_ast.preprocessed_function import PreprocessedFunction
from my_ast.preprocessed_variable import PreprocessedVariable


class PreprocessedClass(PreprocessedDeclaration):
    def __init__(self, name: str):
        super().__init__(name)
        self.methods: List[PreprocessedFunction] = []
        self.attributes: List[PreprocessedVariable] = []
        self.classes: List[PreprocessedClass] = []
        self.type_usages: List[Declaration] = []

    def append_method(self, name: str):
        self.methods.append(PreprocessedFunction(name))

    def append_attribute(self, name: str):
        self.attributes.append(PreprocessedVariable(name))

    def append_class(self, nested_class: 'PreprocessedClass'):  # 'type' -> in case of type(self)
        self.classes.append(nested_class)


class PreprocessedClassCollector(ast.NodeVisitor):
    def __init__(self):
        self.classes: List[PreprocessedClass] = []
        self.class_list: List[int] = []
        self.class_nest_class_map: Dict[PreprocessedClass, List[ast.ClassDef]] = {}

    def append_class(self, node: ast.ClassDef):
        self.class_list.append(len(self.classes))
        preprocessed_class = PreprocessedClass(node.name)
        self.classes.append(preprocessed_class)
        self.handle_nested_class(node, preprocessed_class)
        for member in node.body:
            if isinstance(member, (ast.FunctionDef, ast.AsyncFunctionDef)):
                self.append_method(member)
            elif isinstance(member, ast.Assign):
                self.append_attribute(member)
            elif isinstance(member, ast.ClassDef):
                self.append_nested_class(preprocessed_class, member)
            elif isinstance(member, ast.Pass):
                pass
            elif isinstance(member, ast.Expr) and hasattr(member, 'value') and isinstance(member.value, ast.Constant) \
                    and hasattr(member.value, "value") and isinstance(member.value.value, str):
                pass  # TODO: documentation comment
            else:
                assert False, "Unknown class member: " + str(type(member))

    def handle_nested_class(self, node: ast.ClassDef, preprocessed_class: PreprocessedClass):
        for parent_class in self.class_nest_class_map:
            if node in self.class_nest_class_map[parent_class]:
                parent_class.append_class(preprocessed_class)
                self.class_nest_class_map[parent_class].remove(node)

    def class_processed(self):
        del self.class_list[-1]

    def append_method(self, node: ast.FunctionDef):
        self.classes[self.class_list[-1]].append_method(node.name)
        if node.name == '__init__':
            self.visit(node)

    def append_attribute(self, node: ast.Assign):
        for attribute in node.targets:
            if hasattr(attribute, 'id'):  # class variable
                self.classes[self.class_list[-1]].append_attribute(attribute.id)
            elif hasattr(attribute, 'attr') and has_attr(attribute, 'value.id') and attribute.value.id == 'self':
                self.classes[self.class_list[-1]].append_attribute(attribute.attr)

    def append_nested_class(self, node: PreprocessedClass, member: ast.ClassDef):
        if node in self.class_nest_class_map:
            self.class_nest_class_map[node].append(member)
        else:
            self.class_nest_class_map[node] = [member]

    def visit_Assign(self, node: ast.Assign):
        self.append_attribute(node)
        self.generic_visit(node)
