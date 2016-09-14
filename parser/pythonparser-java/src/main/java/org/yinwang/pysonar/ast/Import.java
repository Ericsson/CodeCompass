package org.yinwang.pysonar.ast;

import org.jetbrains.annotations.NotNull;
import org.yinwang.pysonar.Analyzer;
import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar.State;
import org.yinwang.pysonar.types.Type;

import java.util.List;
import java.util.Set;
import org.yinwang.pysonar.types.ModuleType;


public class Import extends Node {

    public List<Alias> names;


    public Import(List<Alias> names, String file, int start, int end) {
        super(file, start, end);
        this.names = names;
        addChildren(names);
    }


    @NotNull
    @Override
    public Type transform(@NotNull State s) {
        for (Alias a : names) {
            Type mod = Analyzer.self.loadModule(a.name, s);
            if (mod == null) {
                Analyzer.self.putProblem(this, "Cannot load module");
                if (a.name.size() > 0) {
                    this.name = a.name.get(0).id;
                    if (a.asname != null) {
                        s.insert(a.asname.id, this, Type.UNKNOWN, Binding.Kind.MODULEREF);
                    } else {
                        s.insert(a.name.get(0).id, this, Type.UNKNOWN, Binding.Kind.MODULEREF);
                    }
                }
            } else if (a.asname != null) {
                Set<Binding> bs = Analyzer.self.moduleTable.table.get(
                        ((ModuleType) mod).qname);
                if (bs != null) {
                    Binding moduleBinding = bs.iterator().next();
                    Analyzer.self.putRef(a.asname, moduleBinding);
                    s.update(a.asname.id, moduleBinding);
                }
            }
        }
        return Type.CONT;
    }


    @NotNull
    @Override
    public String toString() {
        return "<Import:" + names + ">";
    }

}
