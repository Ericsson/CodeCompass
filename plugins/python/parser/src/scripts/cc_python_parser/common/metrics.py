import datetime
from time import time


class PythonParserMetrics:
    def __init__(self, project: str):
        self.project: str = project
        self.file_count: int = 0
        self.line_count: int = 0
        self.parse_time: float = 0
        self.function_count: int = 0
        self.class_count: int = 0
        self.import_count: int = 0
        self.ast_count: int = 0
        # type metrics

    def add_file_count(self, file_count: int = 1):
        self.file_count += file_count

    def add_line_count(self, line_count: int = 1):
        self.line_count += line_count

    def start_parsing(self):
        self.parse_time = time()

    def stop_parsing(self):
        self.parse_time = time() - self.parse_time

    def add_function_count(self, function_count: int = 1):
        self.function_count += function_count

    def add_class_count(self, class_count: int = 1):
        self.class_count += class_count

    def add_import_count(self, import_count: int = 1):
        self.import_count += import_count

    def add_ast_count(self, ast_count: int = 1):
        self.ast_count += ast_count

    def init(self, project: str):
        self.project = project
        self.file_count = 0
        self.line_count = 0
        self.parse_time = 0
        self.function_count = 0
        self.class_count = 0
        self.import_count = 0
        self.ast_count = 0
        # type metrics

    def __add__(self, other):
        if not isinstance(other, PythonParserMetrics):
            return self
        new = self.__copy__()
        new.file_count += other.file_count
        new.line_count += other.line_count
        new.parse_time += other.parse_time
        new.function_count += other.function_count
        new.class_count += other.class_count
        new.import_count += other.import_count
        new.ast_count += other.ast_count
        return new

    def __iadd__(self, other):
        return self.__add__(other)

    def __copy__(self):
        copy = PythonParserMetrics(self.project)
        copy.file_count = self.file_count
        copy.line_count = self.line_count
        copy.parse_time = self.parse_time
        copy.function_count = self.function_count
        copy.class_count = self.class_count
        copy.import_count = self.import_count
        copy.ast_count = self.ast_count
        return copy

    def __deepcopy__(self, memodict={}):
        return self.__copy__()

    def __str__(self):
        return f"""
        Metrics:
        =============
        Project: {self.project}
        File count: {self.file_count}
        Line count: {self.line_count}
        Parse time: {datetime.timedelta(seconds = self.parse_time)}
        Function count: {self.function_count}
        Class count: {self.class_count}
        Import count: {self.import_count}
        Ast count: {self.ast_count}
        """

    def __repr__(self):
        return self.__str__()


metrics: PythonParserMetrics = PythonParserMetrics('')
