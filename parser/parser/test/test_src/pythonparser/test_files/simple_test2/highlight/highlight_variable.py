"""Highlight test of variables

   This module provide a test cases for check syntax highlight of variables.
"""

var = 42

def L():
  """Testing local scope"""

  var = "string literal"
  print(var)
  
  var = ("tuple literal", 42)
  if var == (42,):
    print(var)


def E():
  """Testing nonlocal scope"""

  var = "string literal"

  def inner():
    print(var)


def LE():
  """Testing local scope in enclosed function"""

  var = "string literal"

  def inner():
    var = ("tuple literal", 42)
    if var == (42,):
      print(var)


def G():
  """Testing global scope"""

  print(var)

  if var == (42,):
    print(var)


def EG():
  """Testing global scope in enclosed function"""

  def inner():
    
    if var == (42,):
      print(var)


def GWithAssigment():
  """Testing global scope with assigment"""

  global var

  var = "string literal"
  print(var)

  var = ("tuple literal", 42)
  if var == (42,):
    print(var)


def EGWithAssigment():
  """Testing global scope with assigment in enclosed function"""

  def inner():
    global var

    var = "string literal"
    print(var)

    var = ("tuple literal", 42)
    if var == (42,):
      print(var)
