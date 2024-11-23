class Base:
    DEBUG_MODE = False
    users = []

    def __init__(self) -> None:
        pass

    def foo(self):
        pass

    def bar(self):
        print("bar")

        def test():
            pass

        test()

class Derived(Base):
    def __init__(self) -> None:
        pass

class Derived2(Derived, Base):
    def __init__(self) -> None:
        pass

base = Base()

def func():
    class A:
        z = 2
