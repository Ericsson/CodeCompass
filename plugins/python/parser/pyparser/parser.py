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

def parseProject(root_path, venv_path, sys_path, n_proc):
    config = ParserConfig(root_path=root_path)
    log(f"Parsing project: {root_path}")
    log_config(config)

    try:
        if venv_path:
            jedi.create_environment(venv_path, safe = config.safe_env)
            config.venv_path = venv_path
            log(f"{bcolors.OKGREEN}Using virtual environment: {venv_path}")
        else:
            venv_path = None

        if sys_path:
            log(f"{bcolors.OKGREEN}Using additional syspath: {sys_path}")
        
        config.project = jedi.Project(path = root_path, environment_path = venv_path, added_sys_path = sys_path)

    except:
        log(f"{bcolors.FAIL}Failed to use virtual environment: {venv_path}")
        if config.stack_trace:
            traceback.print_exc()

    py_files = []
    
    log(f"{bcolors.OKGREEN}Using {n_proc} process to parse project")

    for root, dirs, files in os.walk(root_path):
        for file in files:
            p = os.path.join(root, file)
            ext = os.path.splitext(p)[1]
            
            if ext and ext.lower() == '.py':
                py_files.append(p)

    if config.venv_path:
        py_files = filter(lambda e : not(e.startswith(config.venv_path)), py_files)

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
    hashmap[node.id] = node

