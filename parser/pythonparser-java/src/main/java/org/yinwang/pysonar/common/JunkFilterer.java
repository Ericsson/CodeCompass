package org.yinwang.pysonar.common;

import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.yinwang.pysonar.Analyzer;
import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar.ast.Node;
import org.yinwang.pysonar.types.Type;
import org.yinwang.pysonar.types.UnionType;

public class JunkFilterer {

    private Set<Node> nodes;
    private HashMap<Node, Binding> baseBindings;

    public JunkFilterer(boolean includeBuiltins, boolean includeJunks) {
        nodes = new HashSet<>();
        baseBindings = new HashMap<>();
    }

    public void filter() {
        filterBindings();
        filterNodes();
        setRefs();
        removeInvalidRefs();
        setBaseBindings();
    }

    private void filterBindings() {
        Map<Node, List<Binding>> bindings = new LinkedHashMap<>();
        for (Binding b : Analyzer.self.allBindings) {

            if (!JunkDefinition.isBuiltin(b) && !JunkDefinition.isJunk(b)) {
                if (!bindings.containsKey(b.node)) {
                    LinkedList<Binding> list = new LinkedList<>();
                    list.add(b);
                    bindings.put(b.node, list);
                } else {
                    bindings.get(b.node).add(b);
                }
            }
        }

        Analyzer.self.allBindings.clear();
        
        for (Map.Entry<Node, List<Binding>> e : bindings.entrySet()) {
            
            if (e.getValue().size() == 1) {
                Analyzer.self.allBindings.add(e.getValue().get(0));
            } else {
                Binding b = generateUnionBinding(e.getValue());
                Analyzer.self.allBindings.add(b);
            }
        }
    }

    private Binding generateUnionBinding(List<Binding> bindings) {
        LinkedList<Type> types = new LinkedList<>();
        Set<Node> refs = new HashSet<>();
        boolean definition = false;

        for (Binding binding : bindings) {
            types.add(binding.type);
            refs.addAll(binding.refs);
            if(binding.isDefinitionBinding()){
                definition = true;
            }
        }

        Binding ret = new Binding(bindings.get(0).name, bindings.get(0).node,
                new UnionType(types.toArray(new Type[types.size()])), 
                bindings.get(0).kind, false);
        if(definition){
            ret.markAsDefinitionBinding();
        }
        ret.refs = refs;
        ret.copyLocationInfo(bindings.get(0));

        return ret;
    }

    private void filterNodes() {
        for (Node n : Analyzer.self.getReferences().keySet()) {
            if (!JunkDefinition.isBuiltin(n) && !JunkDefinition.isJunk(n)) {
                nodes.add(n);
            }
        }

        for (Binding b : Analyzer.self.allBindings) {
            nodes.add(b.node);
        }
    }

    private void setRefs() {
        Analyzer.self.references.clear();
        
        for (Binding b : Analyzer.self.allBindings) {
            for (Node n : b.refs) {
                addRef(n, b);
            }
        }
    }

    private boolean skippableRef(Node n) {
        return JunkDefinition.isJunk(n);
    }

    private void setBaseBindings() {
        for (Binding b : Analyzer.self.allBindings) {
            baseBindings.put(b.node, b);
        }
    }

    private void addRef(Node n, Binding b) {
        if (skippableRef(n)) {
            return;
        }

        if (Analyzer.self.references.containsKey(n)) {
            Analyzer.self.references.get(n).add(b);
        } else {
            LinkedList<Binding> list = new LinkedList<>();
            list.add(b);
            Analyzer.self.references.put(n, list);
        }
    }

    private void removeInvalidRefs() {
        LinkedList<Node> toRemove = new LinkedList<>();

        for (Node n : Analyzer.self.references.keySet()) {
            if (!nodes.contains(n)) {
                toRemove.add(n);
            }
        }
        for (Node n : toRemove) {
            Analyzer.self.references.remove(n);
        }
    }

    /**
     * @return the nodes
     */
    public Set<Node> getNodes() {
        return nodes;
    }

    /**
     * @return the bindings
     */
    public List<Binding> getBindings() {
        return Analyzer.self.allBindings;
    }

    /**
     * @return the refs
     */
    public Map<Node, List<Binding>> getRefs() {
        return Analyzer.self.references;
    }

    /**
     * @return the baseBindings
     */
    public HashMap<Node, Binding> getBaseBindings() {
        return baseBindings;
    }
}


//#########################################################################################################################################################################


//package org.yinwang.pysonar.common;
//
//import java.util.HashMap;
//import java.util.HashSet;
//import java.util.LinkedHashMap;
//import java.util.LinkedList;
//import java.util.List;
//import java.util.Map;
//import java.util.Set;
//import org.yinwang.pysonar.Analyzer;
//import org.yinwang.pysonar.Binding;
//import org.yinwang.pysonar.ast.Node;
//import org.yinwang.pysonar.db.data.PythonAstNode;
//import org.yinwang.pysonar.types.Type;
//import org.yinwang.pysonar.types.UnionType;
//
//public class JunkFilterer {
//
//    private Set<Node> validNodes;
//    private Set<Node> junkNodes;
//
//    private LinkedList<Binding> validBindings;
//    private LinkedList<Binding> builtinBindings;
//    private LinkedList<Binding> junkBindings;
//
//    private Set<Node> nodes;
//    private List<Binding> bindings;
//    private Map<Node, List<Binding>> refs;
//
//    private HashMap<Node, Binding> baseBindings;
//
//    private boolean includeBuiltins;
//    private boolean includeJunks;
//
//    public JunkFilterer(boolean includeBuiltins, boolean includeJunks) {
//        this.includeBuiltins = includeBuiltins;
//        this.includeJunks = includeJunks;
//        validNodes = new HashSet<>();
//        junkNodes = new HashSet<>();
//        validBindings = new LinkedList<>();
//        builtinBindings = new LinkedList<>();
//        junkBindings = new LinkedList<>();
//        bindings = new LinkedList<>();
//        nodes = new HashSet<>();
//        refs = new HashMap<>();
//        baseBindings = new HashMap<>();
//    }
//
//    public void filter() {
//        filterBindings();
//        filterNodes();
//        setBindings();
//        setNodes();
//        setRefs();
//        removeInvalidRefs();
////        removeInvalidNodes();
//        setBaseBindings();
//    }
//
//    private void filterBindings() {
//        Map<Node, List<Binding>> bindings = new LinkedHashMap<>();
//        for (Binding b : Analyzer.self.allBindings) {
//
//            if (JunkDefinition.isBuiltin(b)) {
//                builtinBindings.add(b);
//            } else if (JunkDefinition.isJunk(b)) {
//                junkBindings.add(b);
//                Analyzer.self.ds.getErrorLogger().addJunkBinding(b, "Junk binding");
//            } else {
//                if (!bindings.containsKey(b.node)) {
//                    LinkedList<Binding> list = new LinkedList<>();
//                    list.add(b);
//                    bindings.put(b.node, list);
//                } else {
//                    bindings.get(b.node).add(b);
//                }
//            }
//        }
//
//        for (Map.Entry<Node, List<Binding>> e : bindings.entrySet()) {
//            if (e.getValue().size() == 1) {
//                validBindings.add(e.getValue().get(0));
//            } else {
//                Binding b = generateUnionBinding(e.getValue());
//                validBindings.add(b);
//            }
//        }
//    }
//
//    private Binding generateUnionBinding(List<Binding> bindings) {
//        LinkedList<Type> types = new LinkedList<>();
//        Set<Node> refs = new HashSet<>();
//
//        for (Binding binding : bindings) {
//            types.add(binding.type);
//            refs.addAll(binding.refs);
//        }
//
//        Binding ret = new Binding(bindings.get(0).name, bindings.get(0).node,
//                new UnionType(types.toArray(new Type[types.size()])), bindings.get(0).kind);
//        ret.refs = refs;
//        ret.copyLocationInfo(bindings.get(0));
//
//        return ret;
//    }
//
//    private void filterNodes() {
//        for (Node n : Analyzer.self.getReferences().keySet()) {
//            filterNode(n);
//        }
//
//        // add nodes from cleaned allBindings, node is not junk (guaranteed by junk definition)
//        for (Binding b : validBindings) {
//            validNodes.add(b.node);
//        }
//        for (Binding b : junkBindings) {
//            junkNodes.add(b.node);
//            Analyzer.self.ds.getErrorLogger().addJunkNode(b.node, "Junk basebinding");
//        }
//        // we do NOT store builtin nodes
//    }
//
//    private void filterNode(Node n) {
//        if (JunkDefinition.isBuiltin(n)) {
//            // do nothing
//        } else if (JunkDefinition.isJunk(n)) {
//            junkNodes.add(n);
//            Analyzer.self.ds.getErrorLogger().addJunkNode(n, "Junk node");
//        } else {
//            validNodes.add(n);
//        }
//    }
//
//    private void setBindings() {
//        bindings.addAll(validBindings);
//
//        if (includeBuiltins) {
//            bindings.addAll(builtinBindings);
//        }
//        if (includeJunks) {
//            bindings.addAll(junkBindings);
//        }
//    }
//
//    private void setNodes() {
//        nodes.addAll(validNodes);
//
//        if (includeJunks) {
//            for (Node junkNode : junkNodes) {
//                junkNode.markAsJunk();
//            }
//            nodes.addAll(junkNodes);
//        }
//    }
//
//    private void setRefs() {
//        for (Binding b : bindings) {
//            for (Node n : b.refs) {
//                addRef(n, b);
//            }
//        }
//    }
//
//    private boolean insertableRef(Node n) {
//        if (JunkDefinition.isJunk(n)) {
//            if (includeJunks) {
//                return true;
//            } else {
//                return false;
//            }
//        } else {
//            return true;
//        }
//    }
//
//    private void setBaseBindings() {
//        for (Binding b : bindings) {
//            baseBindings.put(b.node, b);
//        }
//    }
//
//    private void addRef(Node n, Binding b) {
//        if (!insertableRef(n)) {
//            return;
//        }
//        if (refs.containsKey(n)) {
//            refs.get(n).add(b);
//        } else {
//            LinkedList<Binding> list = new LinkedList<>();
//            list.add(b);
//            refs.put(n, list);
//        }
//    }
//
//    private void removeInvalidRefs() {
//        LinkedList<Node> toRemove = new LinkedList<>();
//
//        for (Node n : refs.keySet()) {
//            if (!nodes.contains(n)) {
//                toRemove.add(n);
//            }
//        }
//        for (Node n : toRemove) {
//            refs.remove(n);
//        }
//    }
//
//    /**
//     * @return the nodes
//     */
//    public Set<Node> getNodes() {
//        return nodes;
//    }
//
//    /**
//     * @return the bindings
//     */
//    public List<Binding> getBindings() {
//        return bindings;
//    }
//
//    /**
//     * @return the refs
//     */
//    public Map<Node, List<Binding>> getRefs() {
//        return refs;
//    }
//
//    /**
//     * @return the baseBindings
//     */
//    public HashMap<Node, Binding> getBaseBindings() {
//        return baseBindings;
//    }
//}