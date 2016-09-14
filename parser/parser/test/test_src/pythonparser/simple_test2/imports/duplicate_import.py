"""This module is for testing duplicate occure of package's name in the import
    syntax.
"""


import imports.dummies.dummy1
import imports.dummies.dummy2

imports.dummies.dummy1.do_nothing1()
imports.dummies.dummy2.do_nothing2()
#imports.dummies.dummy1.do_nothing()

from imports.dummies.dummy3 import do_nothing3
from imports.dummies.dummy4 import do_nothing4

do_nothing3()
do_nothing4()

