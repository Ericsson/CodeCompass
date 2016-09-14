"""The porpuse of this module is testing the definition resolving of overriden
   methods. Moreover it is good for checking the InfoTree Overrides, Overriden by,
   Inherits from and Inherits by options."""

import overriden_method_import 

def talk(*args):
    pass

class Thing:

    def __init__(self):
        self.talk = 42

    def foo(self):
        print(self.talk)

class Creature:

    def __init__(self, name):
        self.name = name

    def talk(self):
        raise NotImplementedError("Subclass must implement abstract method")


class Animal(Creature):

    def talk(self):
        return '...'


class Cat(Animal):

    def talk(self):
        return 'Meow!'


class Dog(Animal):

    def talk(self):
        return 'Woof! Woof!'


class Lion(Animal):

    def talk(self, param1, param2):
        return "Rawr!!!"


class Elephant(Animal):

    def talk(self, *args, **kwargs):
        return "Toot."


class Crow(Animal):

    def talk(self, csor=1):
        return "Tweet."



animals = [ Cat('Missy'),
            Cat('Mr. Mistoffelees'),
            Dog('Lassie') ]


#The talk call definicions:
#resolved: Cat, Dog
#possible: Animal, Creature, Cat, Crow, Dog, Elephant, overriden_method_import
# overriden_methods.talk
for animal in animals:
    print animal.name + ': ' + animal.talk()

#The talk call definicions:
#resolved: Cat, Dog
#possible: Animal, Creature, Cat, Crow, Dog, Elephant, overriden_method_import
# overriden_methods.talk
def called_talk_with_0_param(animals):
    for animal in animals:
        print animal.name + ': ' + animal.talk()

#The talk call definicions:
#resolved: -
#possible: Animal, Creature, Cat, Crow, Dog, Elephant, overriden_method_import
# overriden_methods.talk
def uncalled_talk_with_0_param(animals):
    for animal in animals:
        if isinstance(animal, Dog) or isinstance(animal, Crow):
            print animal.name + ': ' + animal.talk()

#The talk call definicions:
#resolved: Lion
#possible: Elephant, Lion
# overriden_methods.talk
def called_talk_with_2_param(animal):
    print animal.talk(42, 42)
        
#The talk call definicions:
#resolved: -
#possible: Elephant, Lion
# overriden_methods.talk  
def uncalled_talk_with_2_param(animal):
    print animal.talk(42, 42)
    

called_talk_with_0_param(animals)
called_talk_with_2_param(Lion("The king"))

#resolved: overriden_method_import
#possible: Animal, Creature, Cat, Crow, Dog, Elephant, overriden_method_import
# overriden_methods.talk
overriden_method_import.talk()
