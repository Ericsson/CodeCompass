"""This module is for testing multiple binding.

   In the end of the exec_foo function, the foo() call can invoke either the
   imported and the top-level functions, depending on the param value.
"""

pred = True

def foo():
  pass

if(pred):
  from multiple_bindings.imported_foo import foo
  foo()    # invoke the imported foo
else:
  foo()    # invoke the top-level foo

foo()      # can invoke either imported or top-level foo