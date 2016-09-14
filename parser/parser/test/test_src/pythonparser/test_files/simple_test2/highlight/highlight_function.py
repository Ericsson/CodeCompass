"""Highlight test of functions

   This module provide a test cases for check syntax highlight of functions.
"""

def func():
  pass


def funcParam1(param):
  pass


def funcParam2(param1, param2):
  pass


funcPtr = func
funcPtr()

funcPtr = funcParam1
funcPtr("string literal")
funcPtr(func)


funcParam2(func, funcParam1)


class FuncStorager():
  def memberFunc():
    pass

  def memberFuncParam1(param):
   pass


  def memberFuncParam2(param1, param2):
    pass

fs = FuncStorager()
fs.memberFunc()
fs.memberFuncParam1(funcParam1)

funcParam2(fs.memberFuncParam1, fs.memberFuncParam2)