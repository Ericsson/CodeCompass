import jedi
from hashlib import sha1

config = {
    "root_path": None,
    "venv_path": None,
    "project": None
}

def log(msg):
    print(f"[PythonParser] {msg}")

def project_config(root_path, venv_path, sys_path):
    
    config["root_path"] = root_path

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
        log(f"Failed to use virtual environment: {venv_path}")

def parse(path):
    result = {
        "status": "none",
        "nodes": [],
        "imports": []
    }

    if config["venv_path"] and path.startswith(config["venv_path"]):
        return result

    with open(path) as f:
        log(f"Parsing: {path}")
        source = f.read()
        script = jedi.Script(source, path=path, project=config["project"])

        nodes = {}
        result["status"] = "full"

        for x in script.get_names(references = True, all_scopes = True):
            defs = x.goto(follow_imports = True, follow_builtin_imports = True)
            
            if len(defs) > 0:
                refid = min(list(map(lambda x : hashName(x), defs)))
            else:
                refid = hashName(x)
                reportMissingDefinition(x, result)

            putInMap(nodes, getNodeInfo(x, refid, defs))

            for d in defs:
                putInMap(nodes, getNodeInfo(d, refid))
                
                if d.module_path and not str(d.module_path).startswith(config["root_path"]):
                    result["imports"].append(str(d.module_path))

    result["nodes"] = list(nodes.values())

    if len(result["nodes"]) == 0:
        result["status"] = "none"

    return result

def getNodeInfo(name, refid, defs = []):
    node = {}
    node["id"] = hashName(name)
    node["ref_id"] = refid
    node["module_name"] = name.module_name
    node["module_path"] = name.module_path
    node["full_name"] = name.full_name if name.full_name else ""

    pos = getNamePosInfo(name)
    node.update(pos) # merge pos dictionary

    node["type"] = name.type
    node["is_definition"] = name.is_definition()
    node["file_id"] = getFileId(name)
    node["type_hint"] = getNameTypeHint(name)
    node["is_builtin"] = name.in_builtin_module() or any(list(map(lambda x : x.in_builtin_module(), defs)))

    return node

def getNameTypeHint(name):
    hint = ""
    try:
        res = name.get_type_hint()
        hint = res if res else ""
    except:
        pass

    return hint

def getNamePosInfo(name):
    pos = {
        "line_start": 0,
        "line_end": 0,
        "column_start": 0,
        "column_end": 0,
        "value": ""
    }

    if name.get_definition_start_position():
        pos["line_start"] = name.get_definition_start_position()[0]
        pos["line_end"] = name.get_definition_end_position()[0]
        pos["column_start"] = name.get_definition_start_position()[1] + 1
        pos["column_end"] = name.get_definition_end_position()[1] + 1

        if pos["line_start"] == pos["line_end"]:
            pos["value"] = name.get_line_code()[pos["column_start"]-1:pos["column_end"]-1]
        else:
            pos["value"] = name.get_line_code()[pos["column_start"]-1:]

    return pos

def reportMissingDefinition(name, result):
    if name.is_definition() and name.type == 'module' and getNamePosInfo(name)["line_start"] > 0:
        log(f"Missing {name.description}")
        result["status"] = "partial"

def putInMap(hashmap, node):
    hashmap[node["id"]] = node

def hashName(name):
    pos = getNamePosInfo(name)
    s = f"{name.module_path}|{pos['line_start']}|{pos['line_end']}|{pos['column_start']}|{pos['column_end']}".encode("utf-8")
    hash = int(sha1(s).hexdigest(), 16) & 0xffffffffffffffff
    return hash

def fnvHash(str):
  hash = 14695981039346656037

  for c in str:
    hash ^= ord(c)
    hash *= 1099511628211

  # see: https://stackoverflow.com/questions/20766813/how-to-convert-signed-to-unsigned-integer-in-python
  return hash & 0xffffffffffffffff

def getFileId(name):
    return fnvHash(str(name.module_path))