#Independent classes with the same atrs
class Independent_1:
  l1_1_oa = 42

  def __init__(self):
    self.l1_1_ia = 42

class Independent_2(Independent_1):
  l1_2_oa = 42

  def __init__(self):
    Independent_1.__init__(self)
    self.l1_2_ia = 42

    self.l1_1_ia = "reassigned" 


#Class hierarchy for testing modifield mangled name
class L1_1:
  l1_1_oa = 42
  name = 'hi'

  def __init__(self):
    self.l1_1_ia = 42

class L1_2:
  l1_2_oa = 42
  name = 'hej'

  def __init__(self):
    self.l1_2_ia = 42

class L2_1:
  l2_1_oa = 42

  def __init__(self):
    self.l2_1_ia = 42

class L2_2(L1_1, L1_2):
  l2_2_oa = 42

  def __init__(self):
    L1_2.__init__(self)
    L1_1.__init__(self)
    
    self.l2_2_ia = 42

class L3_1(L2_1, L2_2):
  l3_1_oa = 42

  def __init__(self):
    L2_2.__init__(self)
    L2_1.__init__(self)
    
    self.l3_1_ia = 42

    self.name = 'hello'

class L4_1(L3_1):
  l4_1_oa = 42

  def __init__(self):
    L3_1.__init__(self)
    
    self.l4_1_ia = 42

    print self.l1_1_oa
    print self.l1_1_ia

    print self.l1_2_oa
    print self.l1_2_ia

    print self.l2_1_oa
    print self.l2_1_ia

    print self.l2_2_oa
    print self.l2_2_ia

    print self.l3_1_oa
    print self.l3_1_ia

    print self.l4_1_oa
    print self.l4_1_ia

    print self.name

  def reassign_attrs(self):
    self.l1_1_oa = "reassigned"
    self.l1_1_ia = "reassigned"

    self.l1_2_oa = "reassigned"
    self.l1_2_ia = "reassigned"
    
    self.l2_1_oa = "reassigned"
    self.l2_1_ia = "reassigned"
    
    self.l2_2_oa = "reassigned"
    self.l2_2_ia = "reassigned"
    
    self.l3_1_oa = "reassigned"
    self.l3_1_ia = "reassigned"
    
    self.l4_1_oa = "reassigned"
    self.l4_1_ia = "reassigned"

    self.name = 'alma'


inst = L4_1()
inst.reassign_attrs()
inst_ind = Independent_2()
