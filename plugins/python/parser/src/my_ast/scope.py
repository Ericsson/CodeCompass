from typing import List, Optional
from abc import abstractmethod, ABC
from uuid import uuid4

from my_ast.base_data import Declaration, Usage
from my_ast.class_data import ClassDeclaration
from my_ast.import_preprocessor import ImportTable
from my_ast.logger import logger
from my_ast.function_data import FunctionDeclaration
from my_ast.type_data import InitVariablePlaceholderType
from my_ast.variable_data import VariableDeclaration


class Scope(ABC):
    def __init__(self, name: str):
        self.name: str = name
        self.variable_declarations: List[VariableDeclaration] = []
        self.function_declarations: List[FunctionDeclaration] = []
        self.class_declarations: List[ClassDeclaration] = []
        # TODO: should use only self.declarations, and filter the others? (slower, but less memory)
        self.declarations: List[Declaration] = []

    def append_variable(self, variable_declaration: VariableDeclaration) -> None:
        self.variable_declarations.append(variable_declaration)
        self.declarations.append(variable_declaration)
        logger.debug('Var: ' + variable_declaration.name + ' (' + str(variable_declaration.file_position) + ') {' +
                     self.get_type() + ' - ' + self.name + '} ' + variable_declaration.get_type_repr())

    def append_function(self, function_declaration: FunctionDeclaration) -> None:
        self.function_declarations.append(function_declaration)
        self.declarations.append(function_declaration)
        logger.debug('Func: ' + function_declaration.name + ' (' + str(function_declaration.file_position) + ') {' +
                     self.get_type() + ' - ' + self.name + '} ' + function_declaration.get_type_repr())

    def append_class(self, class_declaration: ClassDeclaration) -> None:
        self.class_declarations.append(class_declaration)
        self.declarations.append(class_declaration)
        logger.debug('Class: ' + class_declaration.name + ' (' + str(class_declaration.file_position) + ') {' +
                     self.get_type() + ' - ' + self.name + '} ')

    def get_variable(self, usage: Usage) -> Optional[VariableDeclaration]:
        return self.get_variable_by_name(usage.name)

    def get_variable_by_name(self, name: str) -> Optional[VariableDeclaration]:
        variables = [x for x in self.variable_declarations if x.name == name]
        # TODO: fix this assert
        # assert len(variables) <= 1
        if len(variables) > 0:
            return variables[0]
        else:
            return None

    def get_function(self, usage: Usage) -> Optional[FunctionDeclaration]:
        return self.get_function_by_name(usage.name)

    def get_function_by_name(self, name: str) -> Optional[FunctionDeclaration]:
        functions = [x for x in self.function_declarations if x.name == name]
        if len(functions) > 0:
            return functions[-1]
        else:
            return None

    def get_class_by_name(self, name: str) -> Optional[ClassDeclaration]:
        classes = [x for x in self.class_declarations if x.name == name]
        if len(classes) > 0:
            return classes[-1]
        else:
            return None

    @abstractmethod
    def get_type(self) -> str:
        pass


class LifetimeScope(Scope, ABC):
    """Symbols declared inside this scope are deleted after exiting from it."""

    def __init__(self, name: str):
        super().__init__(name)


class ImportScope(ABC):
    """Imported modules are available during this scope."""

    def __init__(self):
        self.import_table: ImportTable = ImportTable()


class GlobalScope(LifetimeScope, ImportScope):
    """The first scope, which lives on with its module."""

    def __init__(self):
        LifetimeScope.__init__(self, '')
        ImportScope.__init__(self)

    def get_type(self) -> str:
        return "global"


class FunctionScope(LifetimeScope, ImportScope):
    """Scope of the functions and methods."""

    def __init__(self, name: str):
        LifetimeScope.__init__(self, name)
        ImportScope.__init__(self)

    def get_type(self) -> str:
        return "function"


class ClassScope(LifetimeScope, ImportScope):
    """Scope of the classes."""

    def __init__(self, name: str):
        LifetimeScope.__init__(self, name)
        ImportScope.__init__(self)
        self.init_placeholders: List[InitVariablePlaceholderType] = []

    def get_type(self) -> str:
        return "class"


class PartialLifetimeScope(LifetimeScope, ABC):
    """Not all of the symbols are deleted after existing from it."""
    pass


class ExceptionScope(PartialLifetimeScope):
    """The exception variable is deleted after the 'except' scope,
    but all the other symbols declared in it, are alive after it."""

    def __init__(self):
        super().__init__('exception')

    def get_type(self) -> str:
        return 'exception'


class LambdaScope(LifetimeScope):
    """Scope of lambdas."""

    def __init__(self):
        super().__init__('lambda_' + uuid4().hex)

    def get_type(self) -> str:
        return "lambda"


class GeneratorScope(LifetimeScope):
    """Scope of generators."""

    def __init__(self):
        super().__init__('generator_' + uuid4().hex)

    def get_type(self) -> str:
        return "generator"


class ConditionalScope(Scope):
    """Scope of if."""

    def __init__(self):
        super().__init__('if')

    def get_type(self) -> str:
        return "conditional"


# TODO: need WhileScope and ForScope?
class LoopScope(Scope):
    """Scope of for and while."""

    def __init__(self):
        super().__init__('loop')

    def get_type(self) -> str:
        return "loop"
