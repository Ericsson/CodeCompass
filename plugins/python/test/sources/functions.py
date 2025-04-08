def hello_world():
    print("Hello, world!")

def runner(func, param1, param2):
    return func(param1,param2)

def mul(a, b):
    return a * b

def mul2(a, b):
    return mul(a,b)

def mul3():
    return mul

mylib = {
    "multiply": mul
}

class MyLib:
    def __init__(self):
        self.multiply = mul

mylib2 = MyLib()

a = mul(4,8)
a2 = runner(mul,4,8)
a3 = mul3()(4,8)
a4 = mylib["multiply"](4,8)
a5 = mylib2.multiply(4,8)

if __name__ == "__main__":
    hello_world()

    print(a)
    print(a2)
    print(a3)
    print(a4)
    print(a5)

    print("----------")

    print(mul(4,8))
    print(runner(mul,4,8))
    print(mul3()(4,8))
    print(mylib["multiply"](4,8))
    print(mylib2.multiply(4,8))

def sign(a: int, b: str) -> None:
    pass

def sign2(
    a: int,
    b: str)  -> None:
    pass

def sign3(
    a: int = 2,
    b: str = "hi")  -> None:
    pass

def sign4( # note
    a: int = 2, # note 2
    b: str = "hi")  -> None: # note 3
    pass

from typing import List, Optional

def annotation(a, b) -> None:
    pass

def annotation2(a,
    b) -> str:
    return "abc"

def annotation3(a,
    b) -> int:
    return 0

def annotation4(a,
    b) -> bool:
    return True

def annotation5(a,
    b) -> List[str]:
    return []

def annotation6(a,
    b) -> Optional[str]:
    pass

def annotation7(a,
    b) -> dict[int, bool]:
    return {}

def local_var():
    a = 2
    for i in range(0,10):
        a += i
