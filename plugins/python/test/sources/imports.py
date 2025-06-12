import classes
import os
from functions import mul

a = mul(4,8)
print(a)
print(mul(4,8))

base = classes.Base()
base.bar()

print("pid", os.getpid())
