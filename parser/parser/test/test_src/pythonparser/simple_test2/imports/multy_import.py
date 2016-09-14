"""This module is for testing multy import syntax."""


import imports.dummies.dummy1, imports.dummies.dummy2 as dum2, imports.dummies.dummy3

imports.dummies.dummy1.do_nothing1()
dum2.do_nothing2()
imports.dummies.dummy3.do_nothing3()


from imports.dummies.dummy4 import do_nothing4, nothing4 as n4

do_nothing4()
n4 = "It's still nothing."