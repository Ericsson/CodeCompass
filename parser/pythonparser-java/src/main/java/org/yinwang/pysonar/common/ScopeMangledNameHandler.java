package org.yinwang.pysonar.common;

import java.util.LinkedList;
import java.util.Collection;
import java.util.List;
import org.jetbrains.annotations.Nullable;
import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar.ast.ClassDef;
import org.yinwang.pysonar.ast.FunctionDef;
import org.yinwang.pysonar.ast.Module;
import org.yinwang.pysonar.ast.Node;

/**
 *
 * @author eadaujv
 */
public class ScopeMangledNameHandler {

    private Collection<Node> nodes;

    public ScopeMangledNameHandler(Collection<Node> nodes) {
        this.nodes = nodes;
    }
    
    public void generateMangledNames(List<Binding> bindingList) {
        for (Binding b : bindingList) {
            generateMangledName(b);
        }
    }

    private void generateMangledName(Binding b) {
        Node node = b.node;
        String mangledname = concatParent(node, node.name);
        b.mangledName = (mangledname != null) ? b.qname + ":" + mangledname : b.qname;
    }

    private String concatParent(Node n, String mangledNameFragment) { // a string jobb oldalara kell egy :
        Node parent = getRelevantParent(n);

        if (n instanceof Module) {
            return mangledNameFragment;
        }

        if (parent == null) {
            return null; // junk node, termintae concatenation
        }

        return concatParent(parent, parent.name + ":" + mangledNameFragment);
    }

    /**
     * Cascade up on the syntax tree, and get the first FunctionDef or ClassDef
     * node before the Module node. If a parent node is junk, continue
     * cascading.
     *
     * @param n
     * @return FunctionDef or ClassDef node before the Module node or Module
     * node, if there are no FunctionDef or ClassDef node before the Module node
     * return the Module node. or null, if n is a Module node
     */
    @Nullable
    private Node getRelevantParent(Node n) {
        if (n == null) {
            return null;
        }

        if (n.parent == null) {
            if (n instanceof Module) {
                return n;
            }
        }

        if ((getParent(n) instanceof FunctionDef)) {
            return getResultFD(n);
        } else if (getParent(n) instanceof ClassDef) {
            return getResultCD(n);
        } else {
            return getRelevantParent(getParent(n)); // not fd or cd, cascade up
        }
    }

    /**
     * FUNCTIONDEF
     */
    private Node getResultFD(Node n) {
        Node node = ((FunctionDef) getParent(n)).name;
        if (node != null) {
            return checkIfSelfFD(n);
        } else {
            return getRelevantParent(getParent(n)); // junk parent node, cascade up
        }
    }

    private Node checkIfSelfFD(Node n) {
        if (n == ((FunctionDef) getParent(n)).name) {
            return getRelevantParent(getParent(n)); // binding base node, cascade up
        } else {
            return checkIfValidNode(((FunctionDef) getParent(n)).name);
        }
    }

    /**
     * CLASSDEF
     */
    private Node getResultCD(Node n) {
        Node node = ((ClassDef) getParent(n)).name;
        if (node != null) {
            return checkIfSelfCD(n);
        } else {
            return getRelevantParent(getParent(n)); // junk parent node, cascade up
        }
    }

    private Node checkIfSelfCD(Node n) {
        if (n == ((ClassDef) getParent(n)).name) {
            return getRelevantParent(getParent(n)); // binding base node, cascade up
        } else {
            return checkIfValidNode(((ClassDef) getParent(n)).name);
        }
    }

    private Node checkIfValidNode(Node n) {
        return (nodes.contains(n)) ? n : null;
    }

    private Node getParent(Node n) {
        return (n.parent != null) ? n.parent : n.getFakeParent();
    }
}
