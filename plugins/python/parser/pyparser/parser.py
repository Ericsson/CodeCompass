import jedi
from hashlib import sha1

def log(msg):
    print(f"[PythonParser] {msg}")

def parse(path):
    log(f"[PythonParser] Parsing: {path}")

    result = {
        "status": "full",
        "nodes": []
    }

    with open(path) as f:
        source = f.read()
        script = jedi.Script(source, path=path)

        nodes = {}

        for x in script.get_names(references = True):
            defs = x.goto(follow_imports=True)
            
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

    node["line_start"] = 0
    node["line_end"] = 0
    node["column_start"] = 0
    node["column_end"] = 0
    node["value"] = ""

    if name.get_definition_start_position():
        node["line_start"] = name.get_definition_start_position()[0]
        node["line_end"] = name.get_definition_end_position()[0]
        node["column_start"] = name.get_definition_start_position()[1] + 1
        node["column_end"] = name.get_definition_end_position()[1] + 1
        node["value"] = name.get_line_code()
    
    node["type"] = name.type
    node["is_definition"] = name.is_definition()
    node["is_builtin"] = name.in_builtin_module()
    node["file_id"] = getFileId(name)

    return node

def putInMap(hashmap, node):
    hashmap[node["id"]] = node

def hashName(name):
    s = f"{name.module_path}|{name.line}|{name.column}".encode("utf-8")
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