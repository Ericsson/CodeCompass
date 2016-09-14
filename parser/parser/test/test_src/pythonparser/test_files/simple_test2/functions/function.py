def testParams(first, second):
  print(first)
  print(second)

def testVararg(*args, **kwargs):
  for count, thing in enumerate(args):
    print '{0}. {1}'.format(count, thing)

  for name, value in kwargs.items():
    print '{0} = {1}'.format(name, value)

class X():

  @classmethod
  def some_class_method(cls):
    pass

  @staticmethod
  def some_static_method(self):
    pass

  def funcWithLocVars(self):
    self.attr = 5
    loc1 = 1
    loc2 = 2
    loc3 = loc1 + loc2

