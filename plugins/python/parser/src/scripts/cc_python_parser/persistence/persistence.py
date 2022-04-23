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
        self.log = open(str(PurePath(__file__).parent.joinpath('persistence_log.txt')), 'w+', encoding='utf-8')

    def __del__(self):
        self.log.close()

    def check_c_persistence(self):
        if self.c_persistence is None:
            return
        assert hasattr(self.c_persistence, 'log_info')
        assert hasattr(self.c_persistence, 'log_warning')
        assert hasattr(self.c_persistence, 'log_error')
        assert hasattr(self.c_persistence, 'log_debug')
        assert hasattr(self.c_persistence, 'persist_file')
        assert hasattr(self.c_persistence, 'persist_variable')
        assert hasattr(self.c_persistence, 'persist_function')
        assert hasattr(self.c_persistence, 'persist_preprocessed_class')
        assert hasattr(self.c_persistence, 'persist_class')
        assert hasattr(self.c_persistence, 'persist_import')

    def log_info(self, message: str):
        if self.c_persistence is not None:
            self.c_persistence.log_info(message)
        else:
            self.log.write(message)

    def log_warning(self, message: str):
        if self.c_persistence is not None:
            self.c_persistence.log_warning(message)
        else:
            self.log.write(message)

    def log_error(self, message: str):
        if self.c_persistence is not None:
            self.c_persistence.log_error(message)
        else:
            self.log.write(message)

    def log_debug(self, message: str):
        if self.c_persistence is not None:
            self.c_persistence.log_debug(message)
        else:
            self.log.write(message)

    def persist_file(self, file: FileDTO) -> None:
        if self.c_persistence is not None:
            self.log_debug(f"Persist file: {file.path}")
            self.c_persistence.persist_file(file)
        else:
            self.log.write(f"Persist file: {file.path}\n")

    def persist_variable(self, declaration: VariableDeclarationDTO) -> None:
        if self.c_persistence is not None:
            self.log_debug(f"Persist var: {declaration.qualified_name}")
            self.c_persistence.persist_variable(declaration)
        else:
            self.log.write(f"Persist var: {declaration.qualified_name}\n")

    def persist_function(self, declaration: FunctionDeclarationDTO) -> None:
        if self.c_persistence is not None:
            self.log_debug(f"Persist func: {declaration.qualified_name}")
            self.c_persistence.persist_function(declaration)
        else:
            self.log.write(f"Persist func: {declaration.qualified_name}\n")

    def persist_preprocessed_class(self, declaration: ClassDeclarationDTO) -> None:
        if self.c_persistence is not None:
            self.log_debug(f"Persist preprocessed class: {declaration.qualified_name}")
            self.c_persistence.persist_preprocessed_class(declaration)
        else:
            self.log.write(f"Persist preprocessed class: {declaration.qualified_name}\n")

    def persist_class(self, declaration: ClassDeclarationDTO) -> None:
        if self.c_persistence is not None:
            self.log_debug(f"Persist class: {declaration.qualified_name}")
            self.c_persistence.persist_class(declaration)
        else:
            self.log.write(f"Persist class: {declaration.qualified_name}\n")

    def persist_import(self, imports: ImportDTO) -> None:
        if self.c_persistence is not None:
            self.log_debug("Persist import")
            for i in imports.imported_modules:
                self.log_debug(f"Persist imported module: {i.qualified_name}")
            for i in imports.imported_symbols:
                self.log_debug(f"Persist imported symbol: {i.qualified_name}")
            self.c_persistence.persist_import(imports)
        else:
            for i in imports.imported_modules:
                self.log.write(f"Persist imported module: {i.qualified_name}\n")
            for i in imports.imported_symbols:
                self.log.write(f"Persist imported symbol: {i.qualified_name}\n")


model_persistence = ModelPersistence(None)


def init_persistence(c_persistence):
    global model_persistence
    model_persistence = ModelPersistence(c_persistence)
