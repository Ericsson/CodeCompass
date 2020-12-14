import os
from copy import copy
from pathlib import PurePath
from typing import Final

from cc_python_parser.common.metrics import metrics, PythonParserMetrics
from cc_python_parser.parse_exception import ParseException
from cc_python_parser.parser import Parser
from cc_python_parser.persistence.persistence import ModelPersistence


def main():
    def directory_exception(path: PurePath) -> bool:
        directory = os.path.basename(os.path.normpath(str(path)))
        return directory.startswith('.') or directory == 'venv'

    def file_exception(path: PurePath) -> bool:
        return False

    code_compass: Final[str] = "/home/rmfcnb/ELTE/Code-Compass-Python-Plugin"
    code_checker: Final[str] = "/home/rmfcnb/ELTE/CodeChecker"
    python_std: Final[str] = "/usr/lib/python3.8"
    manim: Final[str] = "/home/rmfcnb/ELTE/manim"
    numpy: Final[str] = "/home/rmfcnb/ELTE/numpy"
    algo: Final[str] = "/home/rmfcnb/ELTE/TheAlgorithms"
    tensorflow: Final[str] = "/home/rmfcnb/ELTE/tensorflow"

    test: Final[str] = "/home/rmfcnb/ELTE/Test"

    project_roots = [code_compass,
                     code_checker,
                     python_std,
                     manim,
                     numpy,
                     algo,
                     tensorflow]

    project_roots = [test]

    metrics_result = []
    for pr in project_roots:
        metrics.init(pr)
        exception = ParseException(directory_exception, file_exception)
        p = Parser([pr], ModelPersistence(None), exception)
        print(f"Parsing: {pr}")
        p.parse()
        metrics_result.append(copy(metrics))

    all_metrics = PythonParserMetrics("ALL")
    for m in metrics_result:
        print(m)
        all_metrics += m

    print(all_metrics)

    pass


if __name__ == '__main__':
    main()
