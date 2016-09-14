"""Test file for check function calls by function ptr."""


def gammavariate():
    theta = 42

def vonmisesvariate():
    kappa = 42


def _test_generator(func):
    func() # In the first call func is points to gammavariate, in second case
           # to vonmisesvariate.


_test_generator(vonmisesvariate)
_test_generator(gammavariate)