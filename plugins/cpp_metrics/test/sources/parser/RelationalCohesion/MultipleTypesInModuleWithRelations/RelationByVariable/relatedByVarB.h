#ifndef relatedByVarB_H
#define relatedByVarB_H

class relatedByVarA;

class relatedByVarB {
private:
    relatedByVarA* varA;
public:
    relatedByVarB() : varA(nullptr) {}
    void f();
};

#endif
