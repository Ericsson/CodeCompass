class A:
  pass

class B(A):

  b_attr1 = 42

  def __init__(self):
    self.b_attr1 = self.b_attr1 + 1

  def foo(self):
    self.b_attr2 = 42

    a_object = A()
    a_object.a_attr1 = 42

b_object = B()
b_object.b_attr3 = 42