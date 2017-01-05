# module c

print('importted CM')

class Fruit():
    def __init__(self, seeds = 2):
        self.seed = seeds

    def numOfSeeds(self):
        print('Number of seeds in Fruit: ' + str(self.seed))

class Apple(Fruit):
    def __init__(self, seeds = 4, color = 'red'):
        Fruit.__init__(self, seeds)
        self.color = color

    def getColor(self):
        print('Color in Apple: ' + self.color)

class GreenApple(Apple):
    def __init__(self, seeds):
        Apple.__init__(self, seeds, 'green')

    def getColor(self):
        print('GreenApple can only has green color (overridden)')