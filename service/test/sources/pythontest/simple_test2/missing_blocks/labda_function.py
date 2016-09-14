lama = lambda param1, param2: (param1 + param2 * param1) / param2
lama(5 ,4)

lonely_lama = lambda param1, param2: param1 == param2

l = map(lambda x: x + 1, [ n for n in range(10)])