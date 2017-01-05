a = 2

'''Module Doksi szoveg.'''

class Macska():
  """Macska Doksi szoveg.
  Macska Doksi szoveg."""

  farok = 1
  elet = 9

  def start(self):
    print("start() is invoked.")
    self.linker()
    a = 3
    print('start' + str(a))

  def linker(self):
    """Linker Doksi szoveg."""
    print("linker() is invoked.")
    end()
    global a
    a = 'hi'
    print(a)

def end():
  print("end() is invoked.")

def commentedGlobal():
  '''commentedGlobal Doksi szoveg'''
  pass

def rec(i):
  if(i>0):
    return rec(i-1)
  else:
    return 1

j = rec(42)

def f(i):
  if(i>0):
    return 1
  elif(i==0):
    return i
  else:
    return 'hello'

x1 = f(-1)
x2 = f(0)
x3 = f(1)

i = 2
print(i)
i = i + 1
print(i)

cirmos = Macska()
end()
cirmos.start()
end()
print('----------')
a = 2
print(a)
cirmos.start()
print(a)
cirmos.linker()
print(a)

#def nyito(self):
#  return ()
