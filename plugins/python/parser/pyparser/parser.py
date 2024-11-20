import os
import jedi
import multiprocessing
import traceback
from itertools import repeat
from parserlog import log, bcolors, log_config
from asthelper import ASTHelper
from pyname import PYName
from parserconfig import ParserConfig
from nodeinfo import NodeInfo
from parseresult import ParseResult
from pyreference import PYReference
from pybuiltin import PYBuiltin

def parseProject(settings, n_proc):
    config = ParserConfig(
        root_path=settings["root_path"],
        venv_path=settings["venv_path"],
        sys_path=settings["sys_path"],
        debug=settings["debug"],
        stack_trace=settings["stack_trace"],
        type_hint=settings["type_hint"],
        submodule_discovery=settings["submodule_discovery"],
        ast=settings["ast"],
        ast_function_call=settings["ast_function_call"],
        ast_import=settings["ast_import"],
        ast_annotations=settings["ast_annotations"],
        ast_inheritance=settings["ast_inheritance"],
        ast_function_signature=settings["ast_function_signature"]
    )

    log(f"Parsing project: {config.root_path}")
    log_config(config)

    py_files = []
    submodule_map = {}
    for root, dirs, files in os.walk(config.root_path):
        if config.venv_path and root.startswith(config.venv_path):
            continue

        for file in files:
            p = os.path.join(root, file)
            ext = os.path.splitext(p)[1]

            if ext and ext.lower() == '.py':
                py_files.append(p)

            if file == '__init__.py':
                parent_dir = os.path.abspath(os.path.join(root, os.pardir))
                submodule_map[parent_dir] = True

    try:
        if config.venv_path:
            jedi.create_environment(config.venv_path, safe = config.safe_env)
            log(f"{bcolors.OKGREEN}Using virtual environment: {config.venv_path}")
        else:
            config.venv_path = None

        if config.sys_path:
            log(f"{bcolors.OKGREEN}Using additional syspath: {config.sys_path}")

        submodule_sys_path = list(submodule_map.keys())
        if config.submodule_discovery and submodule_sys_path:
            log(f"{bcolors.OKBLUE}Submodule discovery results: {submodule_sys_path}")
            config.sys_path.extend(submodule_sys_path)
        
        config.project = jedi.Project(path = config.root_path, environment_path = config.venv_path, added_sys_path = config.sys_path)

    except:
        log(f"{bcolors.FAIL}Failed to use virtual environment: {config.venv_path}")
        if config.stack_trace:
            traceback.print_exc()

    log(f"{bcolors.OKGREEN}Using {n_proc} process to parse project")

    with multiprocessing.Pool(processes=n_proc) as pool:
        results = pool.starmap(parse, zip(py_files, repeat(config)))
        return results
        
def parse(path: str, config: ParserConfig):
    result: ParseResult = ParseResult(path=path)

    PYBuiltin.findBuiltins(config)

    nodes: dict[int, NodeInfo] = {}
    imports: dict[str, bool] = {}

    with open(path) as f:
        try:
            log(f"Parsing: {path}")
            source = f.read()
            script = jedi.Script(source, path=path, project=config.project)
            names = script.get_names(references = True, all_scopes = True)

            asthelper = ASTHelper(path, source, config)
            pyref = PYReference(config, script, names)

            for e in asthelper.getAnnotations():
                putInMap(nodes, e)

            for x in names:
                defs = pyref.getDefs(x)
                refs = pyref.getFileRefs(x)

                putInMap(nodes, PYName(x).addConfig(config).addDefs(defs, result).addRefs(refs).addASTHelper(asthelper).getNodeInfo())

                for d in defs:
                    if not (d.path):
                        continue

                    # Builtin or library definition
                    if ((config.venv_path and d.path.startswith(config.venv_path)) or
                        not (d.path.startswith(config.root_path))):
                        putInMap(nodes, d.addConfig(config).getNodeInfo())
                        imports[d.path] = True

        except:
            log(f"{bcolors.FAIL}Failed to parse file: {path}")
            if config.stack_trace:
                traceback.print_exc()

    result.nodes = list(nodes.values())
    result.imports = list(imports.keys())

    if len(result.nodes) == 0:
        result.status = "none"

    return result

def putInMap(hashmap: dict[int, NodeInfo], node: NodeInfo):
    if node.id in hashmap:
        return

    hashmap[node.id] = node

