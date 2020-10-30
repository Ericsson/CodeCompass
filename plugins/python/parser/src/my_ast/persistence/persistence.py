from my_ast.persistence.file_dto import FileDTO
from my_ast.persistence.variable_dto import VariableDeclarationDTO
from my_ast.persistence.function_dto import FunctionDeclarationDTO
from my_ast.persistence.class_dto import ClassDeclarationDTO
from my_ast.persistence.import_dto import ImportDTO


class ModelPersistence:
    def __init__(self, c_persistence):
        self.c_persistence = c_persistence
        self.check_c_persistence()

    def check_c_persistence(self):
        if self.c_persistence is None:
            return
        assert hasattr(self.c_persistence, 'persist_file')
        assert hasattr(self.c_persistence, 'persist_variable')
        assert hasattr(self.c_persistence, 'persist_function')
        assert hasattr(self.c_persistence, 'persist_class')
        assert hasattr(self.c_persistence, 'persist_module_import')

    def persist_file(self, file: FileDTO) -> None:
        if self.c_persistence is not None:
            self.c_persistence.perist_file(file)

    def persist_variable(self, declaration: VariableDeclarationDTO) -> None:
        if self.c_persistence is not None:
            self.c_persistence.persist_variable(declaration)

    def persist_function(self, declaration: FunctionDeclarationDTO) -> None:
        if self.c_persistence is not None:
            self.c_persistence.persist_function(declaration)

    def persist_class(self, declaration: ClassDeclarationDTO) -> None:
        if self.c_persistence is not None:
            self.c_persistence.persist_class(declaration)

    def persist_import(self, imports: ImportDTO) -> None:
        if self.c_persistence is not None:
            self.c_persistence.persist_module_import(imports)


model_persistence = ModelPersistence(None)


def init_persistence(c_persistence):
    global model_persistence
    model_persistence = ModelPersistence(c_persistence)
