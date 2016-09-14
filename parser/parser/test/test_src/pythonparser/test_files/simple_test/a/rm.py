# module a
def relative():

  print("I should be imported relativly.")


def listComp():

  return [[miniList[i] for miniList in [[1,2,3],[4,5,6],[7,8,9], \
  None, [10,11,12]] if \
  isinstance(miniList, list)] for i in range(3 \
  )]


def setComp():

  return { y for y \
  in \
  {x for x in 'abracadabra' if x \
  not in 'abc'}                 }


def dictComp():

  return {x: x**2 for x in (2, 4, 6)}


def generator():

  return sum(x*x for x in range(10)\
  )


def delete():
  localVar = "Im dying"
  del localVar



def args_kwargs(*args, **kwargs):
  for count, thing in enumerate(args):
    print '{0}. {1}'.format(count, thing)

  for name, value in kwargs.items():
    print '{0} = {1}'.format(name, value)


args_kwargs('apple', 'banana', 'cabbage', apple = 'fruit', \
            cabbage = 'vegetable')