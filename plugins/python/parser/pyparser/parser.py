import jedi
from hashlib import sha1

config = {
    "env": None,
    "env_path": None
}

def log(msg):
    print(f"[PythonParser] {msg}")

def venv_config(venv_path):
    try:
        config["env"] = jedi.create_environment(venv_path)
        config["env_path"] = venv_path
        log(f"Using virtual environment: {venv_path}")
    except:
        log(f"Failed to use virtual environment: {venv_path}")

def parse(path):
    result = {
        "status": "none",
        "nodes": []
    }

    with open(path) as f:
        if config["env_path"] and path.startswith(config["env_path"]):
            return result

        log(f"Parsing: {path}")
        source = f.read()
        script = jedi.Script(source, path=path, environment=config["env"])

        nodes = {}
        result["status"] = "full"

        for x in script.get_names(references = True, all_scopes = True):
            defs = x.goto(follow_imports = True, follow_builtin_imports = True)
            
            if len(defs) > 0:
                refid = min(list(map(lambda x : hashName(x), defs)))
            else:
                result["status"] = "partial"
                log(f"No definition found for {x.full_name}")
                log(f"{x.full_name}: file = {x.module_path} line = {x.line} column = {x.column}")
                refid = hashName(x)
            
            putInMap(nodes, getNodeInfo(x, refid))

            for d in defs:
                putInMap(nodes, getNodeInfo(d, refid))

    result["nodes"] = list(nodes.values())

    if len(result["nodes"]) == 0:
        result["status"] = "none"

    return result

def getNodeInfo(name, refid):
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
    node["is_builtin"] = name.in_builtin_module()
    node["file_id"] = getFileId(name)

    return node

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
        pos["value"] = name.get_line_code()

    return pos

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