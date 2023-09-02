import os
import json
from copy import copy
from json import JSONDecodeError
from pathlib import PurePath, Path
from typing import List, Final

from cc_python_parser.common.metrics import metrics, PythonParserMetrics
from cc_python_parser.parse_exception import ParseException
from cc_python_parser.parser import Parser
from cc_python_parser.persistence.persistence import ModelPersistence


def read_config() -> List[str]:
    """
    Reads projects to parse from 'config.json', if it exists. Projects can be skipped with the "status" key:
    {
        "projects": [
            {
                "project": "project_path",
                "status": true/false
            }
        ]
    }
    """

    config_file_name: Final[str] = 'config.json'
    project_roots = []
    if Path(config_file_name).exists():
        with open(config_file_name, 'r') as config_file:
            try:
                config = json.load(config_file)
            except JSONDecodeError as e:
                print(f"Invalid JSON file: config.json (line: {e.lineno}, column: {e.colno})")
                return project_roots
            except Exception as e:
                print(f"Error during config.json decoding: {e}")
                return project_roots
            for project in config['projects']:
                if project['status']:
                    project_roots.append(project['project'])
    return project_roots

def main():
    def directory_exception(path: PurePath) -> bool:
        directory = os.path.basename(os.path.normpath(str(path)))
        return directory.startswith('.') or directory == 'venv'

    def file_exception(path: PurePath) -> bool:
        return False

    project_roots = read_config()

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


if __name__ == '__main__':
    main()
