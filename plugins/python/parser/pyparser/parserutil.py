def fnvHash(str):
  hash = 14695981039346656037

  for c in str:
    hash ^= ord(c)
    hash *= 1099511628211

  # see: https://stackoverflow.com/questions/20766813/how-to-convert-signed-to-unsigned-integer-in-python
  return hash & 0xffffffffffffffff
