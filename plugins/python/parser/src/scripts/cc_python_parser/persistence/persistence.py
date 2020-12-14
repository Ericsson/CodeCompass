from pathlib import PurePath

from cc_python_parser.persistence.file_dto import FileDTO
from cc_python_parser.persistence.variable_dto import VariableDeclarationDTO
from cc_python_parser.persistence.function_dto import FunctionDeclarationDTO
from cc_python_parser.persistence.class_dto import ClassDeclarationDTO
from cc_python_parser.persistence.import_dto import ImportDTO


class ModelPersistence:
    def __init__(self, c_persistence):
        self.c_persistence = c_persistence
        self.check_c_persistence()
        self.log = open(str(PurePath(__file__).parent.joinpath('persistence_log.txt')), 'w+')

    def __del__(self):
        self.log.close()

    def check_c_persistence(self):
        if self.c_persistence is None:
            return
        assert hasattr(self.c_persistence, 'persist_file')
        assert hasattr(self.c_persistence, 'persist_variable')
        assert hasattr(self.c_persistence, 'persist_function')
        assert hasattr(self.c_persistence, 'persist_preprocessed_class')
        assert hasattr(self.c_persistence, 'persist_class')
        assert hasattr(self.c_persistence, 'persist_import')

    def persist_file(self, file: FileDTO) -> None:
        if self.c_persistence is not None:
            self.c_persistence.print(f"Persist file: {file.path}")
            self.c_persistence.persist_file(file)
        else:
            self.log.write(f"Persist file: {file.path}\n")

    def persist_variable(self, declaration: VariableDeclarationDTO) -> None:
        if self.c_persistence is not None:
            self.c_persistence.print(f"Persist var: {declaration.qualified_name}")
            self.c_persistence.persist_variable(declaration)
        else:
            self.log.write(f"Persist var: {declaration.qualified_name}\n")

    def persist_function(self, declaration: FunctionDeclarationDTO) -> None:
        if self.c_persistence is not None:
            self.c_persistence.print(f"Persist func: {declaration.qualified_name}")
            self.c_persistence.persist_function(declaration)
        else:
            self.log.write(f"Persist func: {declaration.qualified_name}\n")

    def persist_preprocessed_class(self, declaration: ClassDeclarationDTO) -> None:
        if self.c_persistence is not None:
            self.c_persistence.print(f"Persist preprocessed class: {declaration.qualified_name}")
            self.c_persistence.persist_preprocessed_class(declaration)
        else:
            self.log.write(f"Persist preprocessed class: {declaration.qualified_name}\n")

    def persist_class(self, declaration: ClassDeclarationDTO) -> None:
        if self.c_persistence is not None:
            self.c_persistence.print(f"Persist class: {declaration.qualified_name}")
            self.c_persistence.persist_class(declaration)
        else:
            self.log.write(f"Persist class: {declaration.qualified_name}\n")

    def persist_import(self, imports: ImportDTO) -> None:
        if self.c_persistence is not None:
            self.c_persistence.print("Persist import")
            self.c_persistence.persist_import(imports)
        else:
            self.log.write("Persist import\n")


model_persistence = ModelPersistence(None)


def init_persistence(c_persistence):
    global model_persistence
    model_persistence = ModelPersistence(c_persistence)
