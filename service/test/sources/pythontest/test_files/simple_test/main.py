import   os,  a.a2.a2m
import   sys  as    system


globalVar = 42


class DummyError(a.a2.a2m.SuperOfDummyError):
  """It is an error what you never want to be raised."""


  def __init__(self, message, errors):

    super(DummyError, self).__init__(message)
    self.errors = errors


def with_GlobalFunc(strParam, intParam):

  with open(strParam, 'w') as file:
    str_localVar = "Blablabla bla blabla bla."
    file.write(str_localVar)


def exceptFinal_GlobalFunc():

  try:
    raise DummyError("An error occured.")
    print(globalVar)

    global globalVar

  except NameError as ex:
    int_localVar = os.getpid()
    print(ex)
  except:
    os.getenv("PYTHONPATH")
  finally:
    globalVar = "Just a string."


def reraise_tryexcept_GlobalFunc():

  try:
    raise a.a2.a2m.SuperOfDummyError("Error", 42)
  except:
    raise


def tryexcept_GlobalFunc(strParam):

  try:
    raise DummyError("An error occured.")
    print(globalVar)

    global globalVar

  except(NameError):
    int_localVar = system.maxsize()
  except:
    os.getenv(strParam)
    globalVar = True



bm_globalVar = a.a2.a2m.BM("ctor str param")
bm_globalVar.member1 = 42
bm_globalVar.memberFunc()


with_GlobalFunc("str", int())
exceptFinal_GlobalFunc()
#reraise_tryexcept_GlobalFunc()
tryexcept_GlobalFunc("str")


a.a2.a2m.rel()


tryexcept_GlobalFunc("str")