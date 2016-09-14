package org.yinwang.pysonar.common;

import java.util.Collection;
import org.yinwang.pysonar.Analyzer;
import org.yinwang.pysonar.ast.ClassDef;
import org.yinwang.pysonar.ast.FunctionDef;
import org.yinwang.pysonar.ast.Module;
import org.yinwang.pysonar.ast.Node;

public class ContainerBindingHandler {
    private Collection<Node> nodes;

    public ContainerBindingHandler(Collection<Node> nodes) {
        this.nodes = nodes;
    }
    
    public void generateContainerBindings(Collection<Node> nodes){
        for (Node n : nodes) {
            n.containerBinding = getContainerHash(n);
        }
    }

    public String getContainerHash(Node n) {

        if (n == null) {
            return null;
        }

        if (n.parent == null) {
            if (n instanceof Module) {
                return n.getIdHash();
            } else if (n.getFakeParent() == null) {
//                Analyzer.self.errorLogger.addParentError(
//                        n.getIdHash() + ":" + n.getName() + ":" + n.getLineStart() + ":" + n.getOffset());
                return null; // error: there is no parent of this node
            }
        }

        if ((getParent(n) instanceof FunctionDef)) {
            return getResultFD(n);
        } else if (getParent(n) instanceof ClassDef) {
            return getResultCD(n);
        } else {
            return getContainerHash(getParent(n)); // not fd or cd, cascade up
        }
    }

    /**
     * FUNCTIONDEF
     */
    private String getResultFD(Node n) {
        Node node = ((FunctionDef) getParent(n)).name;
        if (node != null) {
            return checkIfSelfFD(n);
        } else {
            return getContainerHash(getParent(n)); // junk parent node, cascade up
        }
    }

    private String checkIfSelfFD(Node n) {
        if (n == ((FunctionDef) getParent(n)).name) {
            return getContainerHash(getParent(n)); // binding base node, cascade up
        } else {
            return checkIfValidNode(((FunctionDef) getParent(n)).name);
        }
    }

    /**
     * CLASSDEF
     */
    private String getResultCD(Node n) {
        Node node = ((ClassDef) getParent(n)).name;
        if (node != null) {
            return checkIfSelfCD(n);
        } else {
            return getContainerHash(getParent(n)); // junk parent node, cascade up
        }
    }

    private String checkIfSelfCD(Node n) {
        if (n == ((ClassDef) getParent(n)).name) {
            return getContainerHash(getParent(n)); // binding base node, cascade up
        } else {
            return checkIfValidNode(((ClassDef) getParent(n)).name);
        }
    }

    private String checkIfValidNode(Node n) {
        return (nodes.contains(n)) ? n.getIdHash() : null;
    }

    private Node getParent(Node n) {
        return (n.parent != null) ? n.parent : n.getFakeParent();
    }
}
