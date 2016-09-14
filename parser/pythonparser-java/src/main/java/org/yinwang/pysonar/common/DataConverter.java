package org.yinwang.pysonar.common;

import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.yinwang.pysonar.Analyzer;
import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar._;
import org.yinwang.pysonar.ast.Attribute;
import org.yinwang.pysonar.ast.Call;
import org.yinwang.pysonar.ast.ClassDef;
import org.yinwang.pysonar.ast.Name;
import org.yinwang.pysonar.ast.Node;
import org.yinwang.pysonar.db.DataPersister;
import org.yinwang.pysonar.db.data.PythonAstNode;
import org.yinwang.pysonar.db.data.PythonBinding;
import org.yinwang.pysonar.db.data.PythonFunctionCall;
import org.yinwang.pysonar.db.data.PythonReference;
import org.yinwang.pysonar.db.data.PythonUnknown;

/**
 *
 * @author eadaujv
 */
public class DataConverter {

    private Set<Node> nodes;
    private HashMap<String, Node> indexedNodes;
    private List<Binding> bindings;
    private Map<Node, List<Binding>> refs;
    private HashMap<Node, Binding> baseBindings;

    private JunkFilterer jf;
    private ContainerBindingHandler cbh;
    private ScopeMangledNameHandler smnh;
    private BindingMangledNameHandler bmnh;
    private OutputDataFormatter odf;
    private DocumentationSetter ds;

    private DataPersister dp;
    
    private HashSet<String> validBindings;

    public DataConverter(DataPersister dp) {
        indexedNodes = new HashMap<>();
        jf = new JunkFilterer(false, false);
        odf = new OutputDataFormatter();
        this.dp = dp;
    }

    public void processData() {
        //PPlogger.info("Before - junkfilterer -" + _.getGCStats());
        jf.filter();

        nodes = jf.getNodes();
        bindings = jf.getBindings();
        refs = jf.getRefs();
        baseBindings = jf.getBaseBindings();
        setValidBindings();

        //PPlogger.info("Before - indexNodes -" + _.getGCStats());
        indexNodes();

        //PPlogger.info("Setting documentation...");
        ds = new DocumentationSetter(baseBindings, bindings, odf);
        ds.setDocumentation();
        //PPlogger.info("Before - generating mangledNames -" + _.getGCStats());
        //PPlogger.info("Generating manglednames...");
        cbh = new ContainerBindingHandler(getNodes());
        cbh.generateContainerBindings(getNodes());
        smnh = new ScopeMangledNameHandler(getNodes());
        smnh.generateMangledNames(bindings);

        bmnh = new BindingMangledNameHandler(validBindings);

        //PPlogger.info("Before - nodes -" + _.getGCStats());
        PPLogger.info("Processing Nodes...");
        preprocessNodes();
        //PPlogger.info("Before - bindings -" + _.getGCStats());
        PPLogger.info("Processing Bindings...");
        preprocessBindings();
        //PPlogger.info("Before - refs -" + _.getGCStats());
        PPLogger.info("Processing References...");
        preProcessReferences();
        //PPlogger.info("Before - unknowns -" + _.getGCStats());
        PPLogger.info("Processing Unknowns...");
        preprocessUnknowns();
        PPLogger.info("Processing Unresolved nodes...");
        preprocessUnresolveds();
        PPLogger.info("Processing done.");
    }
    
    private void setValidBindings(){
        validBindings = new HashSet<>();
        for (Node n : baseBindings.keySet()) {
            validBindings.add(n.idHash);
        }
    }

    private void setGlobalWritesOnBaseNodes() {
        for (Binding b : bindings) {
            if (b.kind == Binding.Kind.SCOPE) {
                b.node.globalWrite = true;
            }
        }
    }

    private void preprocessNodes() {
        setGlobalWritesOnBaseNodes();

        boolean functionCall;
        for (Node n : getNodes()) {
            functionCall = false;
            n.persistanceAstType = PythonAstNode.getAstType(n, baseBindings, refs);
            if (n.persistanceAstType == Node.AstType.FUNCTION && !baseBindings.containsKey(n)) {
                Call c = Node.getContainerCall(n);
                if (c != null) {
                    dp.persistPythonFunctionCall(new PythonFunctionCall(n.idHash, c.args.size()));
                    functionCall = true;
                }
            }
            PythonAstNode node = new PythonAstNode(n, baseBindings, refs);
            if (functionCall) {
                node.astType = Node.AstType.CALL;
            }
            dp.persistPythonAstNode(node);
        }
    }

    private void preprocessBindings() {
        for (Binding b : bindings) {
            dp.persistPythonBinding(new PythonBinding(b, dp));
        }
    }

    private void preProcessReferences() {
        for (Map.Entry<Node, List<Binding>> ref : refs.entrySet()) {
            for (Binding binding : ref.getValue()) {
                dp.persistPythonReference(new PythonReference(
                        ref.getKey().idHash, binding.getIdHash()));
            }
        }
    }

    /**
     * PreprocessUnknowns must follow preprocessNodes, because of the
     * maybeUnknownNodes persistence check.
     */
    private void preprocessUnknowns() {
        HashSet<String> processedUnknowns = new HashSet<>();
        processUnknownFunctions(processedUnknowns);
        processUnknownAttributes(processedUnknowns);
        processMaybeUnknowns();
    }

    private void processUnknownFunctions(HashSet<String> processedUnknowns) {
        for (Map.Entry<String, LinkedList<Call>> u
                : Analyzer.self.ds.getUnknownFuncCalls().entrySet()) {
            for (Call c : u.getValue()) {
                Node func = c.func;

                String funcNameAsString = u.getKey();
                Name funcName = Node.getNameNode(func);
                String targetName = null;

                if (func instanceof Attribute) {
                    targetName = Attribute.getTargetName(
                            ((Attribute) func).target, null);
                } else {
                    continue;
                }

                if (funcName != null) {
                    persistUnknown(dp, processedUnknowns, targetName, funcNameAsString,
                            PythonUnknown.Kind.FUNCTION, funcName);
                } else {
                    persistUnknown(dp, processedUnknowns, targetName, funcNameAsString,
                            PythonUnknown.Kind.FUNCTION, func);
                }
            }
        }
    }

    private void processUnknownAttributes(HashSet<String> processedUnknowns) {
        for (Map.Entry<String, LinkedList<Attribute>> u
                : Analyzer.self.ds.getUnknownAttrs().entrySet()) {
            for (Attribute attr : u.getValue()) {

                Node attrName = Node.getNameNode(attr);

                String uAttrNameAsString = u.getKey();
                String uTargetName = Attribute.getTargetName(
                        ((Attribute) attr).target, null);

                if (attrName != null) {
                    persistUnknown(dp, processedUnknowns, uTargetName, uAttrNameAsString,
                            PythonUnknown.Kind.ATTRIBUTE, attrName);
                } else {
                    persistUnknown(dp, processedUnknowns, uTargetName, uAttrNameAsString,
                            PythonUnknown.Kind.ATTRIBUTE, attr);
                }
            }
        }
    }

    private void persistUnknown(DataPersister dp, HashSet<String> processedUnknowns,
            String targetName, String attrName, PythonUnknown.Kind kind, Node n) {
        if (!processedUnknowns.contains(n.idHash)) {
            dp.persistPythonUnknown(new PythonUnknown(dp, targetName, attrName, kind, n));
            processedUnknowns.add(n.idHash);
        }
    }

    private void processMaybeUnknowns() {
        for (MaybeUnknownNode n : Analyzer.self.ds.getMaybeUnknownNodes()) {
            Node maybeUnknownNode = n.node;
            Node target = n.target;
            if (dp.isPersisted(maybeUnknownNode)) {
                continue;
            }

            if (target != null) {
                dp.persistPythonUnknown(new PythonUnknown(dp, target.name,
                        maybeUnknownNode.name, n.kind,
                        maybeUnknownNode));
            }
        }
    }

    private void preprocessUnresolveds() {
        for (Name u : Analyzer.self.ds.getAllNames().values()) {
            PythonAstNode node = new PythonAstNode(u, Node.AstType.BINDINGLESS);
            dp.persistPythonBindinglessNode(node);
        }
        UnresolvedCallCalculator uc = new UnresolvedCallCalculator(dp);
        uc.execute();
    }
    
    public Set<Binding> getAttributeDefinitionBindings(ClassDef cd){
        return bmnh.lookupAttributeDefinitionBindings(cd.classType);
    }

    public void indexNodes() {
        for (Node n : nodes) {
            indexedNodes.put(n.idHash, n);
        }
    }

    public boolean isValidNode(Node n) {
        return nodes.contains(n);
    }

    public boolean isValidBinding(Binding b) {
        return baseBindings.containsKey(b.node);
    }

    public boolean isBaseBinding(Node n) {
        return baseBindings.containsKey(n);
    }

    public void addValidNode(Node n) {
        nodes.add(n);
        indexedNodes.put(n.idHash, n);
    }

    public Node getValidNodeById(String id) {
        return indexedNodes.get(id);
    }

    public Set<Node> getNodes() {
        return nodes;
    }

    public Map<Node, List<Binding>> getRefs() {
        return refs;
    }

    public String getBindingMangledName(String qname) {
        return bmnh.getMangledName(qname);
    }
    
    public OutputDataFormatter getOdf() {
        return odf;
    }
}
