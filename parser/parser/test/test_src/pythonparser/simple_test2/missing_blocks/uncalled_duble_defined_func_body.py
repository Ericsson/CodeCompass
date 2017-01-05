"""If a function has more than one definition in the same scope, then only
  the first definition will be resolved.

  This test is for testing this, missing block problem."""

def foo():
  """First definition of foo."""

  bar = 5


def foo():
  """The second definition of foo."""
  var = 69
  print(var)
