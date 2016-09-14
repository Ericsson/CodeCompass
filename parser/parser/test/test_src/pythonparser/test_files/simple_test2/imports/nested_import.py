"""Test for nested imports

  This module provide source for testing nested import syntax.
"""

def foo():
  """Top-level function"""

  import imports.dummies.dummy1
  from imports.dummies.dummy2 import do_nothing2

  imports.dummies.dummy1.do_nothing1()
  do_nothing2()


  def bar():
    """Nested function"""

    import imports.dummies.dummy3
    from imports.dummies.dummy4 import do_nothing4

    imports.dummies.dummy3.do_nothing3()
    do_nothing4()