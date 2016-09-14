/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.yinwang.pysonar.common;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import org.yinwang.pysonar.Analyzer;
import org.yinwang.pysonar.ast.Call;
import org.yinwang.pysonar.ast.Name;
import org.yinwang.pysonar.ast.Node;
import org.yinwang.pysonar.db.DataPersister;
import org.yinwang.pysonar.db.data.PythonFunctionCall;

/**
 *
 * @author eadaujv
 */
public class UnresolvedCallCalculator {

    private DataPersister dp;

    public UnresolvedCallCalculator(DataPersister dp) {
        this.dp = dp;
    }
    
    public void execute(){
        persistUnresolvedCalls();
    }
    
    private void persistUnresolvedCalls(){
        
        // unresolved
        HashMap<Call, List<Name>> ufc = createUnresolvedFunctionCalls();
        for (Map.Entry<Call, List<Name>> unresolvedCall : ufc.entrySet()) {
            
            Name fstPosName = getFirstPositionedName(unresolvedCall.getValue());
            if(isParam(fstPosName)){
                // if the name of the function is not unresolved, we can't 
                // create the function call
                continue;
            }

            Call c = unresolvedCall.getKey();
            persistPythonFunctionCall(fstPosName.idHash, c.args.size());
        }
        
        // unknown
        for (Map.Entry<Node, Integer> e : Analyzer.self.ds.getUnknownCallArgNumbers().entrySet()) {
            persistPythonFunctionCall(e.getKey().idHash, e.getValue());
        }
    }
    
    private boolean isParam(Name n){
        if(n.parent instanceof Call){
            if(((Call)n.parent).args.contains(n)){
                return true;
            }
        }
        return false;
    }
    
    private Name getFirstPositionedName(List<Name> l){
        Name min = l.get(0);
        for (Name name : l) {
            if(name.start < min.start){
                min = name;
            }
        }
        return min;
    }
    
    private void persistPythonFunctionCall(String id, int argNum){
        dp.persistPythonFunctionCall(new PythonFunctionCall(id, argNum));
    }

    private HashMap<Call, List<Name>> createUnresolvedFunctionCalls() {
        HashMap<Call, List<Name>> successorsOfCalls = new HashMap<>();
        for (Name u : Analyzer.self.ds.getAllNames().values()) {
            if (u.parent instanceof Call) {
                Call call = (Call) u.parent;

                if (successorsOfCalls.containsKey(call)) {
                    successorsOfCalls.get(call).add(u);
                } else {
                    List<Name> l = new LinkedList<Name>();
                    l.add(u);
                    successorsOfCalls.put(call, l);
                }
            }
        }
        return successorsOfCalls;
    }
}
