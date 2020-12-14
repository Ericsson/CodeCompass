import ast
from pathlib import PurePath
from typing import Optional, List, Set
from importlib import util  # python 3.4 or above

from cc_python_parser.common.position import Range
from cc_python_parser.common.utils import create_range_from_ast_node


class FileDependency:
    def __init__(self, location: PurePath, module: str, alias: str = None):
        self.location = location
        self.module: str = module
        self.alias: Optional[str] = alias

    def get_file(self) -> str:
        module_parts = self.module.split('.')
        return module_parts[-1] + '.py'


class ModuleImport:
    def __init__(self, name: str, alias: Optional[str] = None):
        self.name: str = name
        self.alias: Optional[str] = alias


class SymbolImport:
    def __init__(self, name: str, alias: Optional[str] = None):
        self.name: str = name
        self.alias: Optional[str] = alias

    def is_all_imported(self) -> bool:
        return self.name == '*'


class Import:
    def __init__(self, path: List[str], module: ModuleImport, imported: List[SymbolImport],
                 location: Optional[PurePath], r: Range, is_local: bool = False):
        self.path: List[str] = path
        self.module: ModuleImport = module
        self.imported: List[SymbolImport] = imported
        self.location: Optional[PurePath] = location
        self.range = r
        self.is_local = is_local

    def is_module_import(self):
        return len(self.imported) == 0


# TODO: need locations of files, and check if modules are among them?
# TODO: handle relative import:
#   '.': from the package (parent directory) of the current module (from .'package' import 'module')
#   from the __init__ (from . import 'symbol')
#   '..': from the grandparent directory of the current module (and so on)
class ImportTable:
    def __init__(self):
        self.modules: List[Import] = []
        self.import_paths: Set[PurePath] = set()

    def append_import(self, node: ast.Import, is_local: bool = False):
        self.modules.extend(self.convert_ast_import_to_import(node, is_local))
        for module in node.names:
            m = None
            try:
                if module.name != '__main__':
                    m = util.find_spec(module.name)
            except (AttributeError, ModuleNotFoundError, ImportError, ValueError):
                pass
            if m is not None and m.origin is not None:
                self.import_paths.add(PurePath(m.origin))

    def append_import_from(self, node: ast.ImportFrom, is_local: bool = False):
        self.modules.extend(self.convert_ast_import_from_to_import(node, is_local))
        m = None
        try:
            if node.level > 0 and node.module is None:
                m = util.find_spec(node.names[0].name)
            elif node.level == 0 or node.module is not None:
                m = util.find_spec(node.module + '.' + node.names[0].name)
            else:
                pass    # print()
        except (AttributeError, ModuleNotFoundError, ImportError, ValueError):
            pass
        finally:
            if m is None:
                try:
                    if node.module != '__main__':
                        m = util.find_spec(node.module)
                except (AttributeError, ModuleNotFoundError, ImportError, ValueError):
                    pass
            if m is not None and m.origin is not None:
                self.import_paths.add(PurePath(m.origin))

    @staticmethod
    def convert_ast_import_to_import(node: ast.Import, is_local: bool = False) -> List[Import]:
        imports = []
        for module in node.names:
            module_parts = module.name.split('.')
            module_name = module_parts[-1]
            module_parts.remove(module_name)
            r = create_range_from_ast_node(node)
            imports.append(ImportTable.create_import(module_parts, module_name, module.asname, [], r, is_local))
        return imports

    @staticmethod
    def convert_ast_import_from_to_import(node: ast.ImportFrom, is_local: bool = False) -> List[Import]:
        assert len(node.names) > 0
        is_module = True
        try:
            if node.level > 0 and node.module is None:
                a = util.find_spec(node.names[0].name)
                if a is None:
                    pass    # print()
            elif (node.level == 0 or node.module is not None) and \
                    util.find_spec(node.module + '.' + node.names[0].name) is None:
                is_module = False
        except (AttributeError, ModuleNotFoundError, ImportError, ValueError):
            # v3.7: before: AttributeError, after: ModuleNotFoundError
            is_module = False
        imports = []
        r = create_range_from_ast_node(node)
        if is_module:  # modules
            if node.module is None:     # relative
                module_path = []
            else:
                module_path = node.module.split('.')
            for module in node.names:
                imports.append(
                    ImportTable.create_import(module_path, module.name, module.asname, [], r, is_local))
        else:  # variables, functions
            imported = []
            for module in node.names:
                imported.append(SymbolImport(module.name, module.asname))
            module_parts = node.module.split('.')
            module_name = module_parts[-1]
            module_parts.remove(module_name)
            imports.append(ImportTable.create_import(module_parts, module_name, None, imported, r, is_local))
        return imports

    @staticmethod
    def create_import(path: List[str], name: str, alias: Optional[str], imported: List[SymbolImport],
                      r: Range, is_local: bool = False) -> Import:
        location = None
        module = path
        module.append(name)
        try:
            if module[-1] == '__main__':        # TODO: util.find_spec finds the module, but has no spec (exception)
                spec = None
            else:
                spec = util.find_spec('.'.join(module))
            if spec is None:
                pass    # print()
                # assert False, "Cannot find module: " + '.'.join(module)
            elif spec.has_location:
                location = spec.origin
        except (AttributeError, ModuleNotFoundError, ImportError, ValueError):
            # v3.7: before: AttributeError, after: ModuleNotFoundError
            pass
        optional_location = None
        if location is not None:
            optional_location = PurePath(location)
        return Import(path, ModuleImport(name, alias), imported, optional_location, r, is_local)

    def get_dependencies(self) -> List[FileDependency]:
        dependencies = []
        for module in self.modules:
            if len(module.path) == 0:
                dependencies.append(FileDependency(module.location, module.module.name, module.module.alias))
            else:
                dependencies.append(
                    FileDependency(
                        module.location, '.'.join(module.path) + '.' + module.module.name, module.module.alias))
        return dependencies
