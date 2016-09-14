import ast
import re
import sys
import codecs

from  json import JSONEncoder
from  ast  import *
from _ast  import *

__version__ = "3.25_8"
# Changelog:
# Import's prefix bugfix

# Is it Python 3?
is_python3 = hasattr(sys.version_info, 'major') and (sys.version_info.major == 3)


class ImportModule(AST):
    """An additional (not standard) ast node, represents the module,
       which wanted to import, inside an ImportFrom Ast nodes."""

    def __init__(self, name):
        self.name = name

class AstEncoder(JSONEncoder):
    def default(self, o):
        if hasattr(o, '__dict__'):
            d = o.__dict__
            # workaround: decode strings if it's not Python3 code
            if not is_python3:
                for k in d:
                    if isinstance(d[k], str):
                        if k == 's':
                            d[k] = lines[d['start']:d['end']]
                        else:
                            d[k] = d[k].decode(enc)
            d['type'] = o.__class__.__name__
            return d
        else:
            return str(o)


enc = 'latin1'
lines = ''

def parse_dump(filename, output, end_mark):
    try:
        if is_python3:
            encoder = AstEncoder()
        else:
            encoder = AstEncoder(encoding=enc)

        tree = parse_file(filename)

        # CC specified modifications
        for node in ast.walk(tree):
            # Increasing by 1 is needed because of CodeCompass clicked position calculator
            if hasattr(node, 'increased_pos_numbers'):
                continue

            if hasattr(node, 'col_offset'):
                node.col_offset += 1
            if hasattr(node, 'end_col_offset'):
                node.end_col_offset += 1
            if hasattr(node, 'start_lineno'):
                node.start_lineno += 1
            if hasattr(node, 'end_lineno'):
                node.end_lineno += 1

            node.increased_pos_numbers = True

        
        encoded = encoder.encode(tree)
        f = open(output, "w")
        f.write(encoded)
        f.close()
    finally:
        # write marker file to signal write end
        f = open(end_mark, "w")
        f.close()


def parse_file(filename):
    global enc, lines
    enc, enc_len = detect_encoding(filename)
    f = codecs.open(filename, 'r', enc)
    lines = f.read()

    # remove BOM
    lines = re.sub(u'\ufeff', ' ', lines)

    # replace the encoding decl by spaces to fool python parser
    # otherwise you get 'encoding decl in unicode string' syntax error
    # print('enc:', enc, 'enc_len', enc_len)
    if enc_len > 0:
        lines = re.sub('#.*coding\s*[:=]\s*[\w\d\-]+',  '#' + ' ' * (enc_len-1), lines)

    f.close()
    return parse_string(lines, filename)


def parse_string(string, filename=None):
    tree = ast.parse(string)

    # Modifications of original ast before starting tree improvements
    for node in ast.walk(tree):
        # The original ast defined type attr has to store, because we 
        # overwrite it with the name of the appropriate ast node's class
        if hasattr(node, 'type'):
            node.original_type = node.type

        # Insert ImportModule (non standart) ast node as value of module atrr
        # in ImportFrom
        if isinstance(node, ImportFrom):
            im = ImportModule(node.module)
            im.lineno = node.lineno
            im.parent_col_offset = node.col_offset

            #TODO: Make an acceptable solution...
            if im.name == None:
                im.name = " " # if we use 'from . import x' syntax

            node.module = im


    improve_ast(tree, string)
    if filename:
        tree.filename = filename
    return tree


# short function for experiments
def p(filename):
    parse_dump(filename, "json1", "end1")


def detect_encoding(path):
    fin = open(path, 'rb')
    prefix = str(fin.read(80))
    encs = re.findall('#.*coding\s*[:=]\s*([\w\d\-]+)', prefix)
    decl = re.findall('#.*coding\s*[:=]\s*[\w\d\-]+', prefix)

    if encs:
        enc1 = encs[0]
        enc_len = len(decl[0])
        try:
            info = codecs.lookup(enc1)
            # print('lookedup: ', info)
        except LookupError:
            # print('encoding not exist: ' + enc1)
            return 'latin1', enc_len
        return enc1, enc_len
    else:
        return 'latin1', -1


#-------------------------------------------------------------
#                   improvements to the AST
#-------------------------------------------------------------
def improve_ast(node, s):
    build_index_map(s)
    improve_node(node, s)


# build global table 'idxmap' for lineno <-> index oonversion
def build_index_map(s):
    global line_starts
    idx = 0
    line_starts = [0]
    while idx < len(s):
        if s[idx] == '\n':
            line_starts.append(idx + 1)
        idx += 1


# convert (line, col) to offset index
def map_idx(line, col):
    return line_starts[line - 1] + col


# convert offset index into (line, col)
def map_line_col(idx):
    line = 0
    for start in line_starts:
        if idx < start:
            break
        line += 1
    col = idx - line_starts[line - 1]
    return (line, col)


def improve_node(node, s):
    if isinstance(node, list):
        for n in node:
            improve_node(n, s)

    elif isinstance(node, AST):

        find_start(node, s)
        find_end(node, s)

        find_startLineno(node) 
        find_endLineno(node)
        find_start_col_offset(node)
        find_end_col_offset(node)
        
        add_missing_names(node, s)
        
        for f in node_fields(node):
            improve_node(f, s)

##########################################

def find_startLineno(node):
    ret = None

    if not isinstance(node, AST):
        raise TypeError("Given parameter is not AST node", node)

    if hasattr(node, 'start_lineno'):
        ret = node.start_lineno
    elif hasattr(node, 'start'):
        node.start_lineno = get_lineno_from_char_pos(node.start)
        ret = node.start_lineno

    return ret
        
def find_endLineno(node):
    ret = None
    
    if not isinstance(node, AST):
        raise TypeError("Given parameter is not AST node", node)

    if hasattr(node, 'end_lineno'):
        ret = node.end_lineno
    elif hasattr(node, 'end'):
        node.end_lineno = get_lineno_from_char_pos(node.end)
        ret = node.end_lineno

    return ret

def find_start_col_offset(node):
    ret = None
    
    if not isinstance(node, AST):
        raise TypeError("Given parameter is not AST node", node)

    if hasattr(node, 'col_offset'):
        return node.col_offset
       
    if hasattr(node, 'start_lineno'):
        global line_starts
        node.col_offset = node.start - line_starts[node.start_lineno]
        ret = node.col_offset
        
    return ret
    
def find_end_col_offset(node):
    ret = None
    
    if not isinstance(node, AST):
        raise TypeError("Given parameter is not AST node", node)

    if hasattr(node, 'end_col_offset'):
        return node.end_col_offset
       
    if hasattr(node, 'end_lineno'):
        global line_starts
        node.end_col_offset = node.end - line_starts[node.end_lineno]
        ret = node.end_col_offset
        
    return ret
        
def get_lineno_from_char_pos(char_pos):
    global line_starts
    for i in range(len(line_starts)-1, -1, -1):
        if(char_pos>=line_starts[i]):
            return i

##########################################

def find_start(node, s):
    ret = None    # default value


    if hasattr(node, 'start'):
        ret = node.start

   
    elif isinstance(node, list):
        if node != []:
            ret = find_start(node[0], s)

   
    elif isinstance(node, Module):
        if node.body != []:
            ret = find_start(node.body[0], s)
        else:
            ret = 0

   
    elif isinstance(node, ImportModule):
        ret = map_idx(node.lineno, start_seq(s[map_idx(node.lineno, node.parent_col_offset):], node.name) + node.parent_col_offset)

    elif isinstance(node, BinOp):
        leftstart = find_start(node.left, s)
        if leftstart != None:
            ret = leftstart
        else:
            ret = map_idx(node.lineno, node.col_offset)


    elif hasattr(node, 'lineno'):
        if node.col_offset >= 0:
            ret = map_idx(node.lineno, node.col_offset)
        else:                           # special case for """ strings
            i = map_idx(node.lineno, node.col_offset)
            while i > 0 and i + 2 < len(s) and s[i:i + 3] != '"""' and s[i:i + 3] != "'''":
                i -= 1
            ret = i


    else:
        return None

    if ret == None and hasattr(node, 'lineno'):
        raise TypeError("got None for node that has lineno", node)

    if isinstance(node, AST) and ret != None:
        node.start = ret

    return ret

def find_alias_start(node, importNode, s):
    if not isinstance(node, alias):
        raise TypeError("given node is not an ast.alias", node)
    if not (isinstance(importNode, Import) or isinstance(importNode, ImportFrom)):
        raise TypeError("given importNode is not ast.Import or ast.ImportFrom")

    if hasattr(node, 'start'):
        return node.start

    node.start = s.find(node.name, find_start(importNode, s) + len("import "))
    return node.start

def find_alias_end(node, importNode, s):
    if not isinstance(node, alias):
        raise TypeError("given node is not an ast.alias", node)
    if not (isinstance(importNode, Import) or isinstance(importNode, ImportFrom)):
        raise TypeError("given importNode is not ast.Import or ast.ImportFrom")

    if hasattr(node, 'end'):
        return node.end

    importNodeStart = find_start(importNode, s)

    if node.asname == None:
        node.end = find_alias_start(node, importNode, s) + len(node.name)
    else:
        startAt = s.find(node.asname, \
            s.find(' as ', find_alias_start(node, importNode, s)))

        node.end = s.find(node.asname, \
        s.find(' as ', find_alias_start(node, importNode, s))) + len(node.asname)


    return node.end


def find_import_end(aliasList, importNode, s):
    if not isinstance(aliasList, list):
        raise TypeError("given aliasList is not a list", aliasList)
    if not (isinstance(importNode, Import) or isinstance(importNode, ImportFrom)):
        raise TypeError("given importNode is not ast.Import or ast.ImportFrom")

    if isinstance(importNode, ImportFrom):
        startFrom = find_end(importNode.module, s)
    else:
        startFrom = find_start(importNode, s)
    
    aliasEnds = list()

    for alias in aliasList:
        aliasEnd = find_alias_end_import(alias, startFrom, s)
        aliasEnds.append(aliasEnd)
        startFrom = aliasEnd

    return max(aliasEnds)

def find_alias_start_import(node, startFrom, s):
    if not isinstance(node, alias):
        raise TypeError("given node is not an ast.alias", node)

    if hasattr(node, 'start'):
        return node.start

    if s.startswith(" import ", startFrom) :
        startFrom += len(" import ")
    elif s.startswith("import ", startFrom) :
        startFrom += len("import ")
    elif s.startswith("from ", startFrom) :
        startFrom += len("from ")

    node.start = s.find(node.name, startFrom)
    return node.start


def find_alias_end_import(aliasNode, startFrom, s):
    if not isinstance(aliasNode, alias):
        raise TypeError("given node is not an ast.alias", aliasNode)

    if hasattr(aliasNode, 'end'):
        return aliasNode.end

    aliasNode.end = find_alias_start_import(aliasNode, startFrom, s) + len(aliasNode.name)
    
    if aliasNode.asname != None:
        aliasNode.asname = str_to_name(s, s.find(' as ', aliasNode.end) + len(' as '))

    return aliasNode.end


def find_end(node, s):
    the_end = None

    if hasattr(node, 'end'):
        return node.end

  
    elif isinstance(node, list):
        if node != []:
            the_end = find_end(node[-1], s)

 
    elif isinstance(node, Module):
        if node.body != []:
            the_end = find_end(node.body[-1], s)
        else:
            the_end = 1

   
    elif isinstance(node, Expr):
        the_end = find_end(node.value, s)

  
    elif isinstance(node, Str):
        i = find_start(node, s)
        while s[i] != '"' and s[i] != "'":
            i += 1

        if i + 2 < len(s) and s[i:i + 3] == '"""':
            q = '"""'
            i += 3
        elif i + 2 < len(s) and s[i:i + 3] == "'''":
            q = "'''"
            i += 3
        elif s[i] == '"':
            q = '"'
            i += 1
        elif s[i] == "'":
            q = "'"
            i += 1
        else:
            print("illegal quote:", i, s[i])
            q = ''

        if q != '':
            the_end = end_seq(s, q, i)

  
    elif isinstance(node, Name):
        the_end = find_start(node, s) + len(node.id)

  
    elif isinstance(node, Attribute):
        the_end = end_seq(s, node.attr, find_end(node.value, s))

   
    elif isinstance(node, FunctionDef):
        the_end = find_end(node.body, s)

  
    elif isinstance(node, Lambda):
        the_end = find_end(node.body, s)

  
    elif isinstance(node, ClassDef):
        the_end = find_end(node.body, s)

    # print will be a Call in Python 3
 
    elif not is_python3 and isinstance(node, Print):
        the_end = start_seq(s, '\n', find_start(node, s))

  
    elif isinstance(node, Call):
        start = find_end(node.func, s)
        if start != None:
            the_end = match_paren(s, '(', ')', start)


    elif isinstance(node, Yield):
        value_end = find_end(node.value, s)
        the_end = value_end if value_end else find_start(node, s) + len('yield')

  
    elif isinstance(node, Return):
        if node.value != None:
            the_end = find_end(node.value, s)
        else:
            the_end = find_start(node, s) + len('return')

  
    elif (isinstance(node, For) or
              isinstance(node, While) or
              isinstance(node, If) or
              isinstance(node, IfExp)):
        if node.orelse != []:
            the_end = find_end(node.orelse, s)
        else:
            the_end = find_end(node.body, s)

  
    elif isinstance(node, Assign) or isinstance(node, AugAssign):
        the_end = find_end(node.value, s)

   
    elif isinstance(node, BinOp):
        the_end = find_end(node.right, s)

 
    elif isinstance(node, BoolOp):
        the_end = find_end(node.values[-1], s)

 
    elif isinstance(node, Compare):
        the_end = find_end(node.comparators[-1], s)


    elif isinstance(node, UnaryOp):
        the_end = find_end(node.operand, s)

 
    elif isinstance(node, Num):
        the_end = find_start(node, s) + len(str(node.n))

  
    elif isinstance(node, List):
        the_end = match_paren(s, '[', ']', find_start(node, s));

  
    elif isinstance(node, Set):
        the_end = match_paren(s, '{', '}', find_start(node, s));

 
    elif isinstance(node, Dict):
        the_end = match_paren(s, '{', '}', find_start(node, s));

  
    elif isinstance(node, Subscript):
        the_end = match_paren(s, '[', ']', find_start(node, s));

 
    elif isinstance(node, Tuple):
        if node.elts != []:
            the_end = find_end(node.elts[-1], s)
        else:
            the_end = match_paren(s, '(', ')', find_start(node, s));

  
    elif ((not is_python3 and isinstance(node, TryFinally)) or
              (is_python3 and isinstance(node, Try))):
        if node.finalbody != []:
            the_end = find_end(node.finalbody, s)
        elif node.orelse != []:
            the_end = find_end(node.orelse, s)
        elif node.handlers != []:
            the_end = find_end(node.handlers, s)
        else:
            the_end = find_end(node.body, s)


    elif not is_python3 and isinstance(node, TryExcept):
        if node.orelse != []:
            the_end = find_end(node.orelse, s)
        elif node.handlers != []:
            the_end = find_end(node.handlers, s)
        else:
            the_end = find_end(node.body, s)

   
    elif isinstance(node, ExceptHandler):
        the_end = find_end(node.body, s)

 
    elif isinstance(node, Pass):
        the_end = find_start(node, s) + len('pass')

  
    elif isinstance(node, Break):
        the_end = find_start(node, s) + len('break')

   
    elif isinstance(node, Continue):
        the_end = find_start(node, s) + len('continue')

  
    elif isinstance(node, Global):
        the_end = start_seq(s, '\n', find_start(node, s))

  
    elif isinstance(node, Import):
        the_end = find_import_end(node.names, node, s)


    elif isinstance(node, ImportFrom):
        the_end = find_import_end(node.names, node, s)

    elif isinstance(node, ImportModule):
        the_end = find_start(node, s) + end_seq(s[node.start:], node.name)

    elif isinstance(node, ast.comprehension):
        concated = node.ifs
        concated.extend([node.target, node.iter])
        ends =  [ val for val in [find_end(n, s) for n in concated] if val != None]
        if ends:
            the_end = max(ends)

   
    elif isinstance(node, ListComp):
        the_end = max(find_end(gen, s) for gen in node.generators)

  
    elif isinstance(node, SetComp):
        the_end = max(find_end(gen, s) for gen in node.generators)

    
    elif isinstance(node, DictComp):
        the_end = max(find_end(gen, s) for gen in node.generators)

   
    elif isinstance(node, GeneratorExp):
        the_end = max(find_end(gen, s) for gen in node.generators)

    
    elif isinstance(node, Raise):
        if is_python3:                

            if node.cause != None:
                the_end = find_end(node.cause, s)
            elif node.exc != None:
                the_end = find_end(node.exc, s)
            else:
                the_end = find_start(node, s) + len('raise')

        else:
               
            if node.tback != None:
                the_end = find_end(node.tback, s)
            elif node.inst != None:
                the_end = find_end(node.inst, s)
            elif node.original_type != None:
                the_end = find_end(node.original_type, s)
            else:
                the_end = find_start(node, s) + len('raise')


    
    elif isinstance(node, With):
        the_end = find_end(node.body[-1], s)

    
    elif isinstance(node, Delete):
        the_end = find_end(node.targets[-1], s)


    else:   # can't determine node end, set to 3 chars after start
        start = find_start(node, s)
        if start != None:
            the_end = start + 3



    if isinstance(node, AST) and the_end != None:
        node.end = the_end

    return the_end


def add_missing_names(node, s):
    
    if hasattr(node, 'extra_attr'):
        return

   
    if isinstance(node, list):
        for n in node:
            add_missing_names(n, s)

   
    elif isinstance(node, ClassDef):
        head = find_start(node, s)
        start = s.find("class", head) + len("class")
        if start != None:
            node.name_node = str_to_name(s, start)
            node._fields += ('name_node',)

   
    elif isinstance(node, FunctionDef):
        # skip to "def" because it may contain decorators like @property
        head = find_start(node, s)
        start = s.find("def", head) + len("def")
        if start != None:
            node.name_node = str_to_name(s, start)
            node._fields += ('name_node',)

        # keyword_start = find_start(node, s)
        # node.keyword_node = str_to_name(s, keyword_start)
        # node._fields += ('keyword_node',)

        if node.args.vararg != None:
            if len(node.args.args) > 0:
                vstart = find_end(node.args.args[-1], s)
            else:
                vstart = find_end(node.name_node, s)
            if vstart != None:
                vname = str_to_name(s, vstart)
                node.vararg_name = vname
        else:
            node.vararg_name = None
        node._fields += ('vararg_name',)

        if node.args.kwarg != None:
            if node.vararg_name != None:
                kstart = find_end(node.vararg_name, s)
                if kstart:
                    kname = str_to_name(s, kstart)
                    node.kwarg_name = kname
            else:
                if len(node.args.args) > 0:
                    kstart = find_end(node.args.args[-1], s)
                else:
                    kstart = find_end(node.vararg_name, s)
                if kstart:
                    kname = str_to_name(s, kstart)
                    node.kwarg_name = kname
        else:
            node.kwarg_name = None
        node._fields += ('kwarg_name',)

   
    elif isinstance(node, Attribute):
        start = find_end(node.value, s)
        if start is not None:
            name = str_to_name(s, start)
            node.attr_name = name
            node._fields = ('value', 'attr_name')  # remove attr for node size accuracy

   
    elif isinstance(node, Compare):
        start = find_start(node, s)
        if start is not None:
            node.opsName = convert_ops(node.ops, s, start)
            node._fields += ('opsName',)


    elif (isinstance(node, BoolOp) or
              isinstance(node, BinOp) or
              isinstance(node, UnaryOp) or
              isinstance(node, AugAssign)):
        if hasattr(node, 'left'):
            start = find_end(node.left, s)
        else:
            start = find_start(node, s)
        if start is not None:
            ops = convert_ops([node.op], s, start)
        else:
            ops = []
        if ops != []:
            node.op_node = ops[0]
            node._fields += ('op_node',)


    elif isinstance(node, Num):
        if isinstance(node.n, int) or (not is_python3 and isinstance(node.n, long)):
            type = 'int'
            node.n = str(node.n)
        elif isinstance(node.n, float):
            type = 'float'
            node.n = str(node.n)
        elif isinstance(node.n, complex):
            type = 'complex'
            node.real = node.n.real
            node.imag = node.n.imag
            node._fields += ('real', 'imag')

        node.num_type = type
        node._fields += ('num_type',)

        
    node.extra_attr = True


#-------------------------------------------------------------
#              utilities used by improve AST functions
#-------------------------------------------------------------

# find a sequence in a string s, returning the start point
def start_seq(s, pat, start = 0):
    try:
        return s.index(pat, start)
    except ValueError:
        return len(s)


# find a sequence in a string s, returning the end point
def end_seq(s, pat, start = 0):
    try:
        return s.index(pat, start) + len(pat)
    except ValueError:
        return len(s)


# find matching close paren from start
def match_paren(s, open, close, start):
    while start < len(s) and s[start] != open:
        start += 1
    if start >= len(s):
        return len(s)

    left = 1
    i = start + 1
    while left > 0 and i < len(s):
        if s[i] == open:
            left += 1
        elif s[i] == close:
            left -= 1
        i += 1
    return i


# convert string to Name
def str_to_name(s, start):
    i = start;
    while i < len(s) and not is_alpha(s[i]):
        i += 1
    name_start = i

    ret = []
    while i < len(s) and is_alpha(s[i]):
        ret.append(s[i])
        i += 1
    name_end = i

    id1 = ''.join(ret)
    if id1 == '':
        return None
    else:
        name = Name(id1, None)
        name.start = name_start
        name.end = name_end
        name.lineno, name.col_offset = map_line_col(name_start)
        return name


def convert_ops(ops, s, start):
    syms = []
    for op in ops:
        if type(op) in ops_map:
            syms.append(ops_map[type(op)])
        else:
            print("[WARNING] operator %s is missing from ops_map, "
                  "please report the bug on GitHub" % op)

    i = start
    j = 0
    ret = []
    while i < len(s) and j < len(syms):
        oplen = len(syms[j])
        if s[i:i + oplen] == syms[j]:
            op_node = Name(syms[j], None)
            op_node.start = i
            op_node.end = i + oplen
            op_node.lineno, op_node.col_offset = map_line_col(i)
            ret.append(op_node)
            j += 1
            i = op_node.end
        else:
            i += 1
    return ret


# lookup table for operators for convert_ops
ops_map = {
    # compare:
    Eq: '==',
    NotEq: '!=',
    LtE: '<=',
    Lt: '<',
    GtE: '>=',
    Gt: '>',
    NotIn: 'not in',
    In: 'in',
    IsNot: 'is not',
    Is: 'is',

    # BoolOp
    Or: 'or',
    And: 'and',
    Not: 'not',
    Invert: '~',

    # bit operators
    BitOr: '|',
    BitAnd: '&',
    BitXor: '^',
    RShift: '>>',
    LShift: '<<',


    # BinOp
    Add: '+',
    Sub: '-',
    Mult: '*',
    Div: '/',
    FloorDiv: '//',
    Mod: '%',
    Pow: '**',

    # UnaryOp
    USub: '-',
    UAdd: '+',
}


# get list of fields from a node
def node_fields(node):
    ret = []
    for field in node._fields:
        if field != 'ctx' and hasattr(node, field):
            ret.append(getattr(node, field))
    return ret


# get full source text where the node is from
def node_source(node):
    if hasattr(node, 'node_source'):
        return node.node_source
    else:
        return None


# utility for getting exact source code part of the node
def src(node):
    return node.node_source[node.start: node.end]


def start(node):
    if hasattr(node, 'start'):
        return node.start
    else:
        return 0


def end(node):
    if hasattr(node, 'end'):
        return node.end
    else:
        return None


def is_alpha(c):
    return (c == '_'
            or ('0' <= c <= '9')
            or ('a' <= c <= 'z')
            or ('A' <= c <= 'Z'))


print("Version: %s"%__version__)

if( len(sys.argv[1:]) > 0):
    p(sys.argv[1])

# p('/Users/yinwang/Code/django/tests/invalid_models/invalid_models/models.py')
# p('/Users/yinwang/Dropbox/prog/pysonar2/tests/test-unicode/test1.py')
# p('/Users/yinwang/Code/cpython/Lib/lib2to3/tests/data/bom.py')
# p('/Users/yinwang/Code/cpython/Lib/test/test_decimal.py')
# p('/Users/yinwang/Code/cpython/Lib/test/test_pep3131.py')
# p('/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/tarfile.py')
# p('/Users/yinwang/Code/cpython/Lib/lib2to3/tests/data/false_encoding.py')
# p('/System/Library/Frameworks/Python.framework/Versions/2.5/lib/python2.5/test/test_marshal.py')
# p('/System/Library/Frameworks/Python.framework/Versions/2.5/lib/python2.5/lib-tk/Tix.py')
