package org.yinwang.pysonar.ast;

import org.jetbrains.annotations.NotNull;
import org.yinwang.pysonar.Analyzer;
import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar.State;
import org.yinwang.pysonar._;
import static org.yinwang.pysonar.ast.Node.transformExpr;
import org.yinwang.pysonar.types.ModuleType;
import org.yinwang.pysonar.types.Type;


public class Module extends Node {

    public Block body;


    public Module(Block body, String file, int start, int end) {
        super(file, start, end);
        this.name = _.moduleName(file);
        this.body = body;
        addChildren(this.body);
        
        Analyzer.self.ds.addModule(file, this); 
    }


    @NotNull
    @Override
    public Type transform(@NotNull State s) {
        ModuleType mt = new ModuleType(getName(), getFile(), Analyzer.self.globaltable);
        s.insert(_.moduleQname(getFile()), this, mt, Binding.Kind.MODULE);
        if (body != null) {
            transformExpr(body, mt.table);
        }
        return mt;
    }


    @NotNull
    @Override
    public String toString() {
        return super.toString();
//        return "(module:" + getFile() + ")";
    }

}
