#ifndef relatedByVarA_H
#define relatedByVarA_H

class relatedByVarB;
class relatedByVarC;

class relatedByVarA {
private:
    relatedByVarB* varB;
    relatedByVarC* varC;
public:
    relatedByVarA() : varB(nullptr), varC(nullptr) {}
    void f();
};

#endif
