import json
import re
import os
from json import JSONDecodeError
from typing import Final
from pathlib import Path

from cc_python_parser.persistence.persistence import ModelPersistence
from cc_python_parser.parse_exception import ParseException

SKIP_FILE_NAME: Final[str] = "skip.json"


def read_skip_file(persistence: ModelPersistence) -> ParseException:
    """
    Reads regexes to skip files and directories from 'skip.json' while parsing, if it exists:
    {
        "files": [],
        "directories": [
            "^[.]",
            "^venv$"
        ]
    }
    """

    ff = lambda p: False
    dd = lambda p: False

    split_path = __file__.split(os.sep)
    del split_path[-1]
    split_path[-1] = SKIP_FILE_NAME
    path = f"{os.sep}{os.path.join(*split_path)}"

    if Path(path).exists():
        with open(path, 'r') as config_file:
            try:
                skip = json.load(config_file)
                if skip['files']:
                    ff = lambda p: any(re.search(f, p) is not None for f in skip['files'])
                if skip['directories']:
                    dd = lambda p: any(re.search(d, p) is not None for d in skip['directories'])
            except JSONDecodeError as e:
                persistence.log_warning(f"Invalid JSON file: {SKIP_FILE_NAME} (line: {e.lineno}, column: {e.colno})")
            except Exception as e:
                persistence.log_warning(f"Error during {SKIP_FILE_NAME} decoding: {e}")
    return ParseException(dd, ff)
