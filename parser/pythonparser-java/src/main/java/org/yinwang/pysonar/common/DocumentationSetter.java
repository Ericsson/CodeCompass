package org.yinwang.pysonar.common;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.yinwang.pysonar.Analyzer;
import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar.ast.ClassDef;
import org.yinwang.pysonar.ast.FunctionDef;
import org.yinwang.pysonar.ast.Module;
import org.yinwang.pysonar.ast.Node;

/**
 *
 * @author eadaujv
 */
public class DocumentationSetter {
    private List<Binding> bindings;
    private HashMap<Node, Binding> baseBindings;
    private OutputDataFormatter odf;
    
    public DocumentationSetter(HashMap<Node, Binding> baseBindings,
            List<Binding> bindings, OutputDataFormatter odf){
        this.baseBindings = baseBindings;
        this.bindings = bindings;
        this.odf = odf;
    }
    
    public void setDocumentation() {
        setFuncDefDoc();
        setClassDefDoc();
        setModuleDoc();
    }
    
    private void setFuncDefDoc(){
        for (FunctionDef f : Analyzer.self.ds.getFd().values()) {
            if (f.getDocString() != null) {
                Binding binding = baseBindings.get(f.name);
                if(binding != null){
                    binding.doc = odf.convertStringToHtml(f.getDocString().toString());
                }
            }
        }
    }
    
    private void setClassDefDoc(){
        for (ClassDef c : Analyzer.self.ds.getCd().values()) {
            if (c.getDocString() != null) {
                Binding binding = baseBindings.get(c.name);
                if(binding != null){
                    binding.doc = odf.convertStringToHtml(c.getDocString().toString());
                }
            }
        }
    }
    
    private void setModuleDoc(){
        for (Binding b : bindings) {
            if(b.kind == Binding.Kind.MODULE){
                Module module = Analyzer.self.ds.getModule(b.node.file);
                if(module != null && module.getDocString() != null){
                    b.doc = odf.convertStringToHtml(module.getDocString().toString());
                }
            }
        }
    }
}
