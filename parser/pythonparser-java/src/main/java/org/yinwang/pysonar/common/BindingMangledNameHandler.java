package org.yinwang.pysonar.common;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.jetbrains.annotations.Nullable;
import org.yinwang.pysonar.Analyzer;
import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar.State;
import org.yinwang.pysonar.ast.ClassDef;
import org.yinwang.pysonar.ast.Name;
import org.yinwang.pysonar.ast.Node;
import org.yinwang.pysonar.types.ClassType;

/**
 *
 * @author eadaujv
 */
public class BindingMangledNameHandler {

    private HashMap<ClassType, ClassHierarchyDataStructure> classes;
    private HashSet<String> validBindings;
    private HashMap<String, String> mangledHashes;

    private static boolean foundfirst = false;

    public BindingMangledNameHandler(HashSet<String> validBindings) {
        this.validBindings = validBindings;
        classes = new HashMap<>();
        mangledHashes = new HashMap<>();
        process();
    }

    private void process() {
        HashMap<Name, ClassDef> cd = Analyzer.self.ds.getCd();

        // Construal nodes represent classes in class hierarchy
        for (ClassDef c : cd.values()) {
            classes.put(c.classType, new ClassHierarchyDataStructure(c.classType));
        }

        // Relation to the classes in class hiearchy
        for (Map.Entry<ClassType, ClassHierarchyDataStructure> e : classes.entrySet()) {
            List<State> supers = e.getKey().table.supers;
            if (supers != null) {
                for (State s : supers) {
                    if (classes.get((ClassType) s.type) != null) {
                        e.getValue().addBase(classes.get((ClassType) s.type));
                    }
                }
            }
        }

        lookupAttributeDefinitions();
    }

    //--------------------------------------------------------------------------
    private void lookupAttributeDefinitions() {
        for (ClassHierarchyDataStructure c : classes.values()) {
            State s = c.getCls().table;

            collectDataFromTable(s.table, c); // class attributes
            collectDataFromTable(c.getCls().instanceAttributes.table, c); // instance attributes
        }
    }

    private void collectDataFromTable(Map<String, Set<Binding>> table, ClassHierarchyDataStructure c) {

        for (Map.Entry<String, Set<Binding>> e : table.entrySet()) {
            Binding b = e.getValue().iterator().next(); // choose arbitrary from binding set
            if (isValidAttribute(b) && b.kind == Binding.Kind.ATTRIBUTE) {
                String mangledname = lookupAttr(c, e.getKey());
                foundfirst = false;
                if (mangledname != null) {
                    mangledHashes.put(b.qname, mangledname);
                }
            }
        }
    }

    @Nullable
    private String lookupAttr(ClassHierarchyDataStructure c, String attr) {
        Binding a = c.getAttrAsBinding(attr);

        if ((!isValidAttribute(a)) && foundfirst) {
            return null;
        }

        if (!c.getBases().isEmpty()) { // van base es benne van
            for (ClassHierarchyDataStructure p : c.getBases()) {
                String res = lookupAttr(p, attr);
                if (res != null) {
                    foundfirst = true;
                    return res;
                }
            }
        }
        return c.getAttr(attr); // van, de csak sajat magaban
    }
    //--------------------------------------------------------------------------

    public Set<Binding> lookupAttributeDefinitionBindings(ClassType ct) {
        ClassHierarchyDataStructure c = classes.get(ct);
        State s = c.getCls().table;

        Set<Binding> attrDefs = new HashSet<>();

        attrDefs.addAll(collectBindingData(s.getAllAttributes(), c)); // class attributes
        attrDefs.addAll(collectBindingData(c.getCls().instanceAttributes.getAllAttributes(), c)); // instance attributes

        return attrDefs;
    }

    private Set<Binding> collectBindingData(Set<Binding> attrs, ClassHierarchyDataStructure c) {
        Set<Binding> attrDefs = new HashSet<>();

        for (Binding b : attrs) {
            if (isValidAttribute(b)) {
                Binding bindingDef = lookupAttrBinding(c, b.name);
                foundfirst = false;
                if (bindingDef != null) {
                    attrDefs.add(bindingDef);
                }
            }
        }
        return attrDefs;
    }

    @Nullable
    private Binding lookupAttrBinding(ClassHierarchyDataStructure c, String attr) {
        Binding a = c.getAttrAsBinding(attr);

        if ((!isValidAttribute(a)) && foundfirst) {
            return null;
        }

        if (!c.getBases().isEmpty()) { // van base es benne van
            for (ClassHierarchyDataStructure p : c.getBases()) {
                Binding res = lookupAttrBinding(p, attr);
                if (isValidAttribute(res)) {
                    foundfirst = true;
                    return res;
                }
            }
        }
        return c.getAttrAsBinding(attr); // van, de csak sajat magaban
    }

    private boolean isValidAttribute(Binding b) {
        if (b == null) {
            return false;
        } else {
            return validBindings.contains(b.node.idHash) && b.kind == Binding.Kind.ATTRIBUTE;
        }
    }

    public String getMangledName(String qname) {
        return mangledHashes.get(qname);
    }
}
