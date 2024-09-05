import os
import jedi
import multiprocessing
import traceback
from itertools import repeat
from parserlog import log, bcolors
from asthelper import ASTHelper
from pyname import PYName

def parseProject(root_path, venv_path, sys_path, n_proc):
    config = {
        "debug": True,
        "root_path": root_path,
        "venv_path": None,
        "project": None
    }

    log(f"Parsing project: {root_path}")

    if config["debug"]:
        log(f"{bcolors.WARNING}Parsing in debug mode!")

    try:
        if venv_path:
            jedi.create_environment(venv_path)
            config["venv_path"] = venv_path
            log(f"Using virtual environment: {venv_path}")
        else:
            venv_path = None

        if sys_path:
            log(f"Using additional syspath: {sys_path}")
        
        config["project"] = jedi.Project(path = root_path, environment_path = venv_path, added_sys_path = sys_path)

    except:
        log(f"{bcolors.FAIL}Failed to use virtual environment: {venv_path}")
        if config["debug"]:
            traceback.print_exc()

    py_files = []
    
    log(f"Using {n_proc} process to parse project")

    for root, dirs, files in os.walk(root_path):
        for file in files:
            p = os.path.join(root, file)
            ext = os.path.splitext(p)[1]
            
            if ext and ext.lower() == '.py':
                py_files.append(p)

    if config["venv_path"]:
        py_files = filter(lambda e : not(e.startswith(config["venv_path"])), py_files)

    with multiprocessing.Pool(processes=n_proc) as pool:
        results = pool.starmap(parse, zip(py_files, repeat(config)))
        return results
        
def parse(path, config):
    result = {
        "path": path,
        "status": "full",
        "nodes": [],
        "imports": []
    }

    nodes = {}

    with open(path) as f:
        try:
            log(f"Parsing: {path}")
            source = f.read()
            script = jedi.Script(source, path=path, project=config["project"])

            asthelper = ASTHelper(source)

            for x in script.get_names(references = True, all_scopes = True):
                defs = x.goto(follow_imports = True, follow_builtin_imports = True)

                putInMap(nodes, PYName(x).addDefs(defs, result).addASTHelper(asthelper).getNodeInfo())

                for d in defs:
                    putInMap(nodes, PYName(d).getNodeInfo())

                    if d.module_path and not str(d.module_path).startswith(config["root_path"]):
                        result["imports"].append(str(d.module_path))

        except:
            log(f"{bcolors.FAIL}Failed to parse file: {path}")
            if config["debug"]:
                traceback.print_exc()

    result["nodes"] = list(nodes.values())

    if len(result["nodes"]) == 0:
        result["status"] = "none"

    return result

def putInMap(hashmap, node):
    hashmap[node["id"]] = node

