# module b
class BM():
  """BM class doc str

     It is multiline doc str:
     line 1.
     line 2.
     last line.
  """

  member1 = list("str")


  def __init__(self, strparam):

    self.member2 = 42
    print(strparam)


  def memberFunc(self):
    """BM's member function doc str"""

    self.member3 = "value"


def raiseTypeError():

  raise TypeError, '... ... ... ... ...'


def iteration():
  for i in range(100):
    pass

  i = "It became a string."

bmo = BM("Object of BM")
bmo.memberFunc()


#raiseTypeError()
iteration()
