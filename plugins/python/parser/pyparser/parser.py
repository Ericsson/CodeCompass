import jedi
from hashlib import sha1

def parse(path):
    print(f"[PythonParser] Parsing: {path}")

    with open(path) as f:
        source = f.read()
        script = jedi.Script(source, path=path)
        
        names_arr = []

        for x in script.get_names(definitions = True, references = True):
            name = {}
            
            name["hash"] = hashName(x)
            name["module_name"] = x.module_name
            name["module_path"] = x.module_path
            name["full_name"] = x.full_name
            name["line_start"] = x.get_definition_start_position()[0]
            name["line_end"] = x.get_definition_end_position()[0]
            name["line"] = x.line
            name["column"] = x.column
            name["column_start"] = x.get_definition_start_position()[1]
            name["column_end"] = x.get_definition_end_position()[1]
            name["type"] = x.type
            name["definition"] = x.is_definition()

            names_arr.append(name)

    return names_arr

def hashName(name):
    s = f"{name.module_path}|{name.line}|{name.column}".encode("utf-8")
    hash = int(sha1(s).hexdigest(), 16) & 0xffffffffffffffff
    return hash

