from hashlib import sha1
from posinfo import PosInfo

def getHashName(path, pos: PosInfo) -> int:
    s = f"{path}|{pos.line_start}|{pos.line_end}|{pos.column_start}|{pos.column_end}".encode("utf-8")
    hash = int(sha1(s).hexdigest(), 16) & 0xffffffffffffffff
    return hash

def fnvHash(str) -> int:
  hash = 14695981039346656037

  for c in str:
    hash ^= ord(c)
    hash *= 1099511628211

  # see: https://stackoverflow.com/questions/20766813/how-to-convert-signed-to-unsigned-integer-in-python
  return hash & 0xffffffffffffffff
