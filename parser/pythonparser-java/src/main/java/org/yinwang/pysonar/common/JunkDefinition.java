package org.yinwang.pysonar.common;

import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar.ast.Node;

public class JunkDefinition {
    
    public static boolean isBuiltin(Node n) {
        return builtinNodeDef(n);
    }

    public static boolean isBuiltin(Binding b) {
        return isBuiltin(b.node);
    }

    public static boolean isJunk(Node n) {
        return junkNodeDef(n);
    }

    public static boolean isJunk(Binding b) {
        return junkBindingDef(b);
    }

    public static boolean junkBindingDef(Binding b) {
        return junkNodeDef(b.node)
                || junkBindingDefBodyMinus(b)
                || junkBindingDefBodyZero(b)
                || junkBindingDefStartEndMinus(b);
    }

    public static boolean junkNodeDef(Node n) {
        return junkNodeDefBodyEndEqualsStart(n)
                || junkNodeDefBodyEndZero(n)
                || junkNodeDefBodyMinus(n)
                || junkNodeDefStartEndMinus(n)
                || junkNodeDefEndZero(n)
                || junkNodeDefFileNull(n);
                //|| junkNodeDefAstTypeIsUnknown(n);
    }
    
    // builtin node definition
    public static boolean builtinNodeDef(Node n) {
        return n.file == null
                && n.getStart() == 0 && n.getStart() == 0
                && n.getLineStart() == 0 && n.getOffset() == 0
                && n.getLineEnd() == 0 && n.getEndOffset() == 0;
    }

    // junk binding definitions
    public static boolean junkBindingDefBodyZero(Binding b) {
        return b.bodyStartColumn == 0 && b.bodyStartLine == 0
                && b.bodyEndColumn == 0 && b.bodyEndLine == 0
                && b.kind != Binding.Kind.MODULE;
    }

    public static boolean junkBindingDefBodyMinus(Binding b) {
        return b.bodyStartColumn == -1 || b.bodyStartLine == -1
                || b.bodyEndColumn == -1 || b.bodyEndLine == -1;
    }

    public static boolean junkBindingDefStartEndMinus(Binding b) {
        return b.start == -1 || b.end == -1;
    }

    // junk node definitions
    public static boolean junkNodeDefStartEndMinus(Node n) {
        return (n.getStart() == -1) || (n.getStart() == -1);
    }

    public static boolean junkNodeDefEndZero(Node n) {
        return n.getEnd() == 0;
    }

    public static boolean junkNodeDefBodyMinus(Node n) {
        return n.getLineStart() == -1 || n.getOffset() == -1
                || n.getLineEnd() == -1 || n.getEndOffset() == -1;
    }

    public static boolean junkNodeDefBodyEndZero(Node n) {
        return n.getLineEnd() == 0 && n.getEndOffset() == 0;
    }

    public static boolean junkNodeDefBodyEndEqualsStart(Node n) {
        return n.getLineEnd() == n.getLineStart()
                && n.getEndOffset() == n.getOffset();
    }

    public static boolean junkNodeDefAstTypeIsUnknown(Node n) {
        return n.getAstType().equals("UNKNOWN");
    }

    private static boolean junkNodeDefFileNull(Node n) {
        return n.file == null;
    }
}
