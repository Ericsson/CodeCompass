package org.yinwang.pysonar.common;

import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar.ast.Attribute;
import org.yinwang.pysonar.ast.Call;
import org.yinwang.pysonar.ast.ClassDef;
import org.yinwang.pysonar.ast.FunctionDef;
import org.yinwang.pysonar.ast.Module;
import org.yinwang.pysonar.ast.Name;
import org.yinwang.pysonar.ast.Node;

/**
 *
 * @author eadaujv
 */
public class DataStorage {

    private HashMap<Name, FunctionDef> fd;
    private HashMap<Name, ClassDef> cd;
    private HashMap<Node, Binding> baseBindings;
    public HashSet<String> classDefHashes;
    public HashMap<Name, Attribute> attributes;
    public HashMap<String, Module> modules;
    private ErrorLogger errorLogger;
    public HashMap<String, Name> allNames;
    private HashMap<Node, Integer> unknownCallArgNumbers;

    // key: attr name, value: call
    private HashMap<String, LinkedList<Call>> unknownFuncCalls;
    // key: attr name, value: attr
    private HashMap<String, LinkedList<Attribute>> unknownAttrs;
    private HashSet<MaybeUnknownNode> maybeUnknownNodes;

    public DataStorage() {
        fd = new HashMap<>();
        cd = new HashMap<>();
        baseBindings = new HashMap<>();
        classDefHashes = new HashSet<>();
        attributes = new HashMap<>();
        modules = new HashMap<>();
        modules = new HashMap<>();
        errorLogger = new ErrorLogger();
        allNames = new HashMap<>();
        unknownCallArgNumbers = new HashMap<>();
        unknownFuncCalls = new HashMap<>();
        unknownAttrs = new HashMap<>();
        maybeUnknownNodes = new HashSet<>();
    }

    public void addFunctionDef(Name n, FunctionDef f) {
        getFd().put(n, f);
    }

    public FunctionDef getFunctionDef(Name n) {
        return getFd().get(n);
    }

    public HashMap<Name, FunctionDef> getFd() {
        return fd;
    }

    public void addClassDef(Name n, ClassDef c) {
        getCd().put(n, c);
        classDefHashes.add(n.idHash);
    }

    public ClassDef getClassDef(Name n) {
        return getCd().get(n);
    }
    
    public HashMap<Name, ClassDef> getCd() {
        return cd;
    }
    
    public void addBaseBinding(Binding b){
        baseBindings.put(b.node, b);
    }
    
    public Binding getBindingByBase(Node n){
        return baseBindings.get(n);
    }

    public void addAttribute(Name attr, Attribute a) {
        attributes.put(attr, a);
    }

    public Attribute getAttribute(Node attr) {
        return attributes.get(attr);
    }

    public void addModule(String n, Module m) {
        modules.put(n, m);
    }

    public Module getModule(String n) {
        return modules.get(n);
    }

    public void addName(String n, Name name) {
        allNames.put(n, name);
    }

    public HashMap<String, Name> getAllNames() {
        return allNames;
    }

    public void removeName(String s) {
        allNames.remove(s);
    }

    public boolean isClassDefByIdHash(String hash) {
        return classDefHashes.contains(hash);
    }

    public void addUnknownCallArg(Node n, Integer i) {
        getUnknownCallArgNumbers().put(n, i);
    }

    public HashMap<Node, Integer> getUnknownCallArgNumbers() {
        return unknownCallArgNumbers;
    }

    public void addUnknownFuncCall(Name n, Call c) {
        if (unknownFuncCalls.containsKey(n.id)) {
            unknownFuncCalls.get(n.id).add(c);
        } else {
            LinkedList<Call> list = new LinkedList<>();
            list.add(c);
            unknownFuncCalls.put(n.id, list);
        }
    }

    public void addUnknownAttribute(Name n, Attribute a) {
        if (unknownAttrs.containsKey(n.id)) {
            unknownAttrs.get(n.id).add(a);
        } else {
            LinkedList<Attribute> list = new LinkedList<>();
            list.add(a);
            unknownAttrs.put(n.id, list);
        }
    }
    
    public void addMaybeUnknownNode(MaybeUnknownNode n){
        maybeUnknownNodes.add(n);
    }

    public HashMap<String, LinkedList<Call>> getUnknownFuncCalls() {
        return unknownFuncCalls;
    }

    public HashMap<String, LinkedList<Attribute>> getUnknownAttrs() {
        return unknownAttrs;
    }

    public HashSet<MaybeUnknownNode> getMaybeUnknownNodes() {
        return maybeUnknownNodes;
    }

    /**
     * @return the errorLogger
     */
    public ErrorLogger getErrorLogger() {
        return errorLogger;
    }
}
