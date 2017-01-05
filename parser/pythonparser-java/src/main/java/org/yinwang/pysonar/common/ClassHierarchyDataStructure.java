package org.yinwang.pysonar.common;

import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import org.jetbrains.annotations.NotNull;
import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar.types.ClassType;

/**
 *
 * @author eadaujv
 */
public class ClassHierarchyDataStructure {

    private ClassType cls;
    private List<ClassHierarchyDataStructure> successors;
    private List<ClassHierarchyDataStructure> bases;

    public ClassHierarchyDataStructure(ClassType cls) {
        this.cls = cls;
        successors = new LinkedList<>();
        bases = new LinkedList<>();
    }

    public void addSuccessor(@NotNull ClassHierarchyDataStructure s) {
            getSuccessors().add(s);
            s.getBases().add(this);
    }

    public void addBase(@NotNull ClassHierarchyDataStructure s) {
            getBases().add(s);
            s.getSuccessors().add(this);
    }

    public String getAttr(String s) {
        Set<Binding> bs = cls.table.table.get(s);
        if (bs != null) {
            return bs.iterator().next().qname;
        }
        return null;
    }
    
    public Binding getAttrAsBinding(String s){
        Set<Binding> bs = cls.table.table.get(s);
        if (bs != null) {
            return bs.iterator().next();
        }
        return null;
    }

    /**
     * @return the cls
     */
    public ClassType getCls() {
        return cls;
    }

    /**
     * @return the successors
     */
    public List<ClassHierarchyDataStructure> getSuccessors() {
        return successors;
    }

    /**
     * @return the bases
     */
    public List<ClassHierarchyDataStructure> getBases() {
        return bases;
    }

}
