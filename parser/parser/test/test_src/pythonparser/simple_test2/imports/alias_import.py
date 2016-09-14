"""This module is for testing aliases in import syntax."""


import imports.dummies.dummy1 as dum1

dum1.do_nothing1()


from imports.dummies.dummy2 import do_nothing2 as doNothing2

doNothing2()