package org.yinwang.pysonar.ast;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.yinwang.pysonar.Analyzer;
import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar.State;
import org.yinwang.pysonar.types.FunType;
import org.yinwang.pysonar.types.InstanceType;
import org.yinwang.pysonar.types.Type;
import org.yinwang.pysonar.types.UnionType;

import java.util.Set;
import java.util.StringTokenizer;

import static org.yinwang.pysonar.Binding.Kind.ATTRIBUTE;
import static org.yinwang.pysonar.ast.Node.transformExpr;
import org.yinwang.pysonar.common.MaybeUnknownNode;
import org.yinwang.pysonar.db.data.PythonUnknown;
import org.yinwang.pysonar.types.ClassType;

public class Attribute extends Node {

    @Nullable
    public Node target;
    @NotNull
    public Name attr;

    public Attribute(@Nullable Node target, @NotNull Name attr, String file, int start, int end) {
        super(file, start, end);
        this.target = target;
        this.attr = attr;
        addChildren(target, attr);
        Analyzer.self.ds.addAttribute(attr, this);
    }

    public void setAttr(State s, @NotNull Type v) {
        Type targetType = transformExpr(target, s);

        if (targetType instanceof UnionType) {
            Set<Type> types = ((UnionType) targetType).types;
            for (Type tp : types) {
                setAttrType(tp, v);
            }
        } else {
            if (target instanceof Name) {
                if (Node.isFirstParamOfConstructor(target)) { // test for "self"
                    ClassDef cd = Node.getContainerClass(attr);
                    if (cd != null) {
                        setAttrType(cd.classType, v);
                    }
                } else if (Node.isFirstParamOfMethod(target)) {  // test for "self"
                    setInstanceAttrType(targetType, v);
                } else {
                    setAttrType(targetType, v);
                }
            } else {
                setAttrType(targetType, v);
            }
        }
    }

    private void addRef(@NotNull Type targetType, @NotNull Set<Binding> bs) {
        for (Binding b : bs) {
            Analyzer.self.putRef(attr, b);
            if (parent != null && parent instanceof Call
                    && b.type instanceof FunType && targetType instanceof InstanceType) {  // method call
                ((FunType) b.type).setSelfType(targetType);
            }
        }
    }

    private void setAttrType(@NotNull Type targetType, @NotNull Type v) {
        if (targetType.isUnknownType()) {
            Analyzer.self.putProblem(this, "Can't set attribute for UnknownType");
            return;
        }
        Set<Binding> bs = targetType.table.lookupAttr(attr.id);
        if (bs != null) {
            addRef(targetType, bs);
        }
        targetType.table.insert(attr.id, attr, v, ATTRIBUTE);

    }

    private void setInstanceAttrType(@NotNull Type targetType, @NotNull Type v) {
        ClassType ct = (ClassType) ((InstanceType) targetType).classType;

        if (targetType.isUnknownType()) {
            return;
        }
        Set<Binding> bs = targetType.table.lookupAttr(attr.id);
        if (bs != null) {
            addRef(targetType, bs);
        } else {
            Set<Binding> bsClass = ct.table.lookupAttr(attr.id);
            if (bs != null) {
                addRef(targetType, bsClass);
            } 
        }

        targetType.table.insertInstanceAttribute(attr.id, attr, v, ATTRIBUTE, ct);
    }

    @NotNull
    @Override
    public Type transform(State s) {
        // the form of ::A in ruby
        if (target == null) {
            return transformExpr(attr, s);
        }

        Type targetType = transformExpr(target, s);
        if (targetType instanceof UnionType) {
            Set<Type> types = ((UnionType) targetType).types;
            Type retType = Type.UNKNOWN;
            for (Type tt : types) {
                retType = UnionType.union(retType, getAttrType(tt));
            }
            return retType;
        } else {
            return getAttrType(targetType);
        }
    }

    private Type getAttrType(@NotNull Type targetType) {
        Set<Binding> bs = targetType.table.lookupAttr(attr.id);
        if (bs == null) {
            Analyzer.self.putProblem(attr, "attribute not found in type: " + targetType);
            addMaybeUnknown(attr, target, (parent instanceof Call));

            if (parent instanceof Call) {
                Analyzer.self.ds.addUnknownCallArg(attr, Node.getCalledParamNum(attr));
            }

            if (targetType.equals(Type.UNKNOWN)) {
                addMaybeUnknown(target, null, (parent instanceof Call));
                Analyzer.self.unknownTypeUsage(this);
            }
            Type t = Type.UNKNOWN;
            t.table.setPath(targetType.table.extendPath(attr.id));
            return t;
        } else {
            addRef(targetType, bs);
            return State.makeUnion(bs);
        }
    }

    private void addMaybeUnknown(Node n, Node t, boolean isCall) {
        if ((n instanceof Name && t instanceof Name)
                || n instanceof Name && t == null) {
            Analyzer.self.ds.addMaybeUnknownNode(new MaybeUnknownNode(n, t,
                    (isCall) ? PythonUnknown.Kind.FUNCTION : PythonUnknown.Kind.ATTRIBUTE));
        }
    }

    public static String getTargetName(Node targetNode, String targetName) {
        if (targetName == null) {
            targetName = "";
        }
        if (targetNode == null) {
            if (targetName.length() > 0) {
                return targetName.substring(0, targetName.length() - 1);
            }
        }

        if (targetNode instanceof Attribute) {
            Attribute a = (Attribute) targetNode;
            return Attribute.getTargetName(a.target, a.attr.name + "." + targetName);
        }

        if (targetNode instanceof Call) {
            Call c = (Call) targetNode;
            if (c.func.name == null) {
                return Attribute.getTargetName(c.func, targetName);
            } else {
                return Attribute.getTargetName(c.func, c.func.name + "." + targetName);
            }
        }

        if (targetName.length() > 0) {
            return targetNode.name + "." + targetName.substring(0, targetName.length() - 1);
        } else {
            return targetNode.name;
        }
    }

    public boolean isFirstAttrSelf() {
        String targetNameConcatenated = getTargetName(target, null);
        if (targetNameConcatenated == null) {
            return false;
        }
        StringTokenizer st = new StringTokenizer(targetNameConcatenated, ".");
        if (st.hasMoreTokens()) {
            return st.nextToken().equals("self");
        }
        return false;
    }

    @NotNull
    @Override
    public String toString() {
        return "<Attribute:" + getStart() + ":" + target + "." + attr.id + ">";
    }

}
