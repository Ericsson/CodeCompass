import imported_counter

# tests.counter.gl
gl = 42

class Counter(object):
  def __str__(self):
    # tests.counter.Counter._name
    # tests.counter.Counter._count
    return self._name + ": " + str(self._count)

  def get_count(self):
    print gl
    # tests.counter.Counter._count
    return self._count

  def set_count(self, count):
    global gl
    # tests.counter.gl
    gl = 84
    # tests.counter.Counter.set_count.count
    # tests.counter.Counter._count
    self._count = count

  def inc(self):
    # tests.counter.Counter.inc.s
    s = "Hello"
    # tests.counter.Counter._count
    self._count += 1
    # tests.counter.Counter._count
    print self._count
    # tests.counter.Counter._count
    self._count += 1
    # tests.counter.Counter._count
    return self._count
  
  def __init__(self, name, start = 0):
    # tests.counter.Counter.__init__.start
    # tests.counter.Counter._count
    self._count = start
    # tests.counter.Counter.__init__.name
    # tests.counter.Counter._name
    self._name = name

class Counter2(Counter):
  def inc2(self):
    # tests.counter.Counter._count
    print self._count
    # tests.counter.Counter._count
    self._count += 1
    # tests.counter.Counter._count
    print self._count
    # tests.counter.Counter._count
    self._count += 1
    # tests.counter.Counter._count
    return self._count

class Counter3(Counter, imported_counter.Counter4):
  def __init__(self):
    super(Counter3, self).__init__("X", 0)
    # tests.counter.Counter3._count
    self._count = 10
    self._alma = 1
    # tests.counter.Counter._name
    print self._name

  def inc2(self):
    # tests.counter.Counter3._count
    self._count += 1
    # tests.counter.Counter3._count
    print self._count
    # tests.counter.Counter3._count
    self._count += 1
    
    self._alma += 2
    print self._alma

    # tests.counter.Counter3._count
    return self._count

def foo():
  # tests.counter.foo.x
  x = 10
  def bar():
    # tests.counter.foo.bar.x
    x = 40

def uncalled_global(counter_instance):
    print counter_instance._name
