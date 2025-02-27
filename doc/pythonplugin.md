# Python Plugin

## Parsing Python projects
Python projects can be parsed by using the `CodeCompass_parser` executable.
See its usage [in a seperate document](/doc/usage.md).

## Python specific parser flags

### Python dependencies
Large Python projects usually have multiple Python package dependencies.
Although a given project can be parsed without installing any of its dependencies, it is strongly recommended
that the required modules are installed in order to achieve a complete parsing.
To install a project's dependencies, create a [Python virtual environment](https://docs.python.org/3/library/venv.html)
and install the necessary packages. 
When parsing a project, specify the virtual environment path so the parser can successfully resolve the dependencies:
```
--venvpath <path to virtual environment>
```

### Type hints
The parser can try to determine Python type hints for variables, expressions and functions.
It can work out type hints such as `Iterable[int]` or `Union[int, str]`.
However, this process can be extremely slow, especially for functions, thus it is disabled by default.
It can be enabled using the `--type-hint` flag.

### Python submodules
Large Python projects can have internal submodules and the parser tries to locate them automatically.
Specifically, it looks for `__init__.py` files and considers those folders modules.
This process is called submodule discovery and can be disabled using the `--disable-submodule-discovery` flag.

You can also add submodules manually by adding those specific paths to the parser's syspath:
```
--syspath <path>
```
For more information, see the [Python syspath docs](https://docs.python.org/3/library/sys.html#sys.path).

### File references
By default, the parser works out references by looking for definitions only - if nodes share the same definition
they are considered references.
However, this method sometimes misses a few references (e.g. local variables in a function).
To extend search for references in a file context, apply the `--file-refs` flag.
Note that using this option can potentially extend the total parsing time.

## Examples of parsing Python projects

### Flask
We downloaded [flask 3.1.0](https://github.com/pallets/flask/releases/tag/3.1.0) source code to `~/parsing/flask/`.
The first step is to create a Python virtual environment and install flask's dependencies.
Create a Python virtual environment and activate it:
```bash
cd ~/parsing/flask/
python3 -m venv venv
source venv/bin/activate
```
Next, we install the required dependencies listed in `pyproject.toml`.
```bash
pip install .
```
Further dependencies include development packages listed in `requirements/dev.txt`.
These can be also installed using `pip`.
```bash
pip install -r requirements/dev.txt
```
Finally, we can run `CodeCompass_parser`.
```bash
CodeCompass_parser \
  -n flask \
  -i ~/parsing/flask/ \
  -w ~/parsing/workdir/ \
  -d "pgsql:host=localhost;port=5432;user=compass;password=pass;database=flask" \
  -f \
  --venvpath ~/parsing/flask/venv/ \
  --label src=~/parsing/flask/
```

### CodeChecker
We downloaded [CodeChecker 6.24.4](https://github.com/Ericsson/codechecker/releases/tag/v6.24.4) source code to `~/parsing/codechecker`.
CodeChecker has an automated way of creating a Python virtual environment and installing dependencies - by running the `venv` target of a Makefile:
```bash
cd ~/parsing/codechecker/
make venv
```
Next, we can run `CodeCompass_parser`.
```bash
CodeCompass_parser \
  -n codechecker \
  -i ~/parsing/codechecker/ \
  -w ~/parsing/workdir/ \
  -d "pgsql:host=localhost;port=5432;user=compass;password=pass;database=codechecker" \
  -f \
  --venvpath ~/parsing/codechecker/venv/ \
  --label src=~/parsing/codechecker/
```

## Troubleshooting
A few errors can occur during the parsing process, these are highlighted in color red.
The stack trace is hidden by default, and can be shown using the `--stack-trace` flag. 

### Failed to use virtual environment
This error can appear if one specifies the `--venvpath` option during parsing.
The parser tried to use the specified virtual environment path, however it failed.

#### Solution
Double check that the Python virtual environment is correctly setup and its
path is correct.
If the error still persists, apply the `--stack-trace` parser option
to view a more detailed stack trace of the error.

### Missing module (file = path line = number)
In this case, the parser tried to parse a given Python file, however it
could not find a definition for a module.
Commonly, the Python file imports another module and the parser cannot locate this module.
If this happens, the Python file is marked *partial* indicating that
a module definition was not resolved in this file.
The error message displays the module name, exact file path and line number
so one can further troubleshoot this problem.

#### Solution
Ensure that the `--venvpath` option is correctly specified and all the required
dependencies are installed in that Python virtual environment.
If the imported module is part of the parsed project, use the `--syspath` option
and specify the directory where the module is located in.

