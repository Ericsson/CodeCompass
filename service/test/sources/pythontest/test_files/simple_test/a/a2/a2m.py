# module a/a2
"""AM module doc str"""

from   b.bm  import   BM
from ..rm    import relative   as  rel


class SuperOfDummyError(Exception):

  def __init__(self, message, errors):
    """SuperOfDummyError's ctor doc str"""

    super(SuperOfDummyError, self).__init__(message)
    self.error = (   (42, None ), "Wow, it realy seems \
      an error."   ,(None)  )

def createGreenApple():
  from   c.cm  import   GreenApple
  myGreenApple = GreenApple(8)
  myGreenApple.numOfSeeds()
  myGreenApple.getColor()

list_expl = list()
list_impl = []


set_expl = set()
set_impl = {42} # to be a set, it should contain at least one element


dict_expl = dict()
dict_impl = {}


str_expl = str()
str_impl = ""


rel()


from   c.cm  import   Apple

myApple = Apple(6, 'green')
myApple.numOfSeeds()
myApple.getColor()

createGreenApple()

from c.macska import cska, ka
import c.williamspear, c.pear