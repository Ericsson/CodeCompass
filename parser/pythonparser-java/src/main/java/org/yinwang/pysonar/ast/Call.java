package org.yinwang.pysonar.ast;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.yinwang.pysonar.*;
import org.yinwang.pysonar.types.*;

import java.util.*;

import static org.yinwang.pysonar.Binding.Kind.ATTRIBUTE;
import static org.yinwang.pysonar.Binding.Kind.CLASS;


public class Call extends Node {

    public FunctionDef functionDefForStack = null;
    public Node func;
    public List<Node> args;
    @Nullable
    public List<Keyword> keywords;
    public Node kwargs;
    public Node starargs;


    public Call(Node func, List<Node> args, @Nullable List<Keyword> keywords,
                Node kwargs, Node starargs, String file, int start, int end)
    {
        super(file, start, end);
        this.func = func;
        this.args = args;
        this.keywords = keywords;
        this.kwargs = kwargs;
        this.starargs = starargs;
        addChildren(func, kwargs, starargs);
        addChildren(args);
        addChildren(keywords);
    }

    public Call(Node func, List<Node> args, @Nullable List<Keyword> keywords,
                Node kwargs, Node starargs, String file, int start, int end, Node parent)
    {
        this(func, args, keywords, kwargs, starargs, file, start, end);
        this.parent = parent;
    }


    /**
     * Most of the work here is done by the static method invoke, which is also
     * used by Analyzer.applyUncalled. By using a static method we avoid building
     * a NCall node for those dummy calls.
     */
    @NotNull
    @Override
    public Type transform(State s) {

        Type fun = transformExpr(func, s);
        List<Type> pos = resolveList(args, s);
        Map<String, Type> hash = new HashMap<>();

        if (keywords != null) {
            for (Keyword kw : keywords) {
                hash.put(kw.arg, transformExpr(kw.value, s));
            }
        }

        Type kw = kwargs == null ? null : transformExpr(kwargs, s);
        Type star = starargs == null ? null : transformExpr(starargs, s);

        if (fun instanceof UnionType) {
            Set<Type> types = ((UnionType) fun).types;
            Type retType = Type.UNKNOWN;
            for (Type ft : types) {
                Type t = resolveCall(ft, pos, hash, kw, star);
                retType = UnionType.union(retType, t);
            }
            return retType;
        } else {
            return resolveCall(fun, pos, hash, kw, star);
        }
    }


    @NotNull
    private Type resolveCall(@NotNull Type fun,
                             List<Type> pos,
                             Map<String, Type> hash,
                             Type kw,
                             Type star)
    {
        if (fun instanceof FunType) {
            FunType ft = (FunType) fun;
            return apply(ft, pos, hash, kw, star, this);
        } else if (fun instanceof ClassType) {
            return new InstanceType(fun, this, pos);
        } else {
            addWarning("calling non-function and non-class: " + fun);
            return Type.UNKNOWN;
        }
    }


    @NotNull
    public static Type apply(@NotNull FunType func,
                             @Nullable List<Type> pos,
                             Map<String, Type> hash,
                             Type kw,
                             Type star,
                             @Nullable Node call)
    {
        Analyzer.self.removeUncalled(func);

        if (func.func != null && !func.func.called) {
            Analyzer.self.nCalled++;
            func.func.called = true;
        }

        Node callToStack = call;
        if (call != null && call instanceof Call )
        {
            callToStack = new Call(((Call) call).func, ((Call) call).args,
                    ((Call) call).keywords, ((Call) call).kwargs,
                    ((Call) call).starargs, ((Call) call).file,
                    ((Call) call).start, ((Call) call).end, call.parent);
            ((Call)callToStack).functionDefForStack = func.func;
        }


        if (func.func == null) {
            // func without definition (possibly builtins)
            return func.getReturnType();
        } else if (call != null && Analyzer.self.inStack(callToStack)) {
            func.setSelfType(null);
            return Type.UNKNOWN;
        }

        if (call != null) {
            Analyzer.self.pushStack(callToStack);
        }

        List<Type> pTypes = new ArrayList<>();

        // Python: bind first parameter to self type
        if (func.selfType != null) {
            pTypes.add(func.selfType);
        } else {
            if (func.cls != null) {
                pTypes.add(func.cls.getCanon());
            }
        }

        if (pos != null) {
            pTypes.addAll(pos);
        }

        bindMethodAttrs(func);

        State funcTable = new State(func.env, State.StateType.FUNCTION);

        if (func.table.parent != null) {
            funcTable.setPath(func.table.parent.extendPath(func.func.name.id));
        } else {
            funcTable.setPath(func.func.name.id);
        }

        Type fromType = bindParams(call, func.func, funcTable, func.func.args,
                func.func.vararg, func.func.kwarg,
                pTypes, func.defaultTypes, hash, kw, star);

        Type cachedTo = func.getMapping(fromType);
        if (cachedTo != null) {
            func.setSelfType(null);
            return cachedTo;
        } else {
            Type toType = transformExpr(func.func.body, funcTable);
            if (missingReturn(toType)) {
                Analyzer.self.putProblem(func.func.name, "Function not always return a value");

                if (call != null) {
                    Analyzer.self.putProblem(call, "Call not always return a value");
                }
            }

            toType = UnionType.remove(toType, Type.CONT);
            func.addMapping(fromType, toType);
            func.setSelfType(null);
            return toType;
        }
    }

@NotNull
    static private Type bindParams(@Nullable Node call,
            @NotNull FunctionDef func,
            @NotNull State funcTable,
            @Nullable List<Node> args,
            @Nullable Name rest,
            @Nullable Name restKw,
            @Nullable List<Type> calledTypes,
            @Nullable List<Type> defaultTypes,
            @Nullable Map<String, Type> hash,
            @Nullable Type kw,
            @Nullable Type star) {
        TupleType fromType = new TupleType();
        int argumentNumber = args == null ? 0 : args.size(); // all arguments (including given, default and keyword)
        int calledTypeSize = calledTypes == null ? 0 : calledTypes.size(); // physically given, non-keyword arguments
        int defaultTypeSize = defaultTypes == null ? 0 : defaultTypes.size(); // defaults in signature
        int nonDefaultNumber = argumentNumber - defaultTypeSize;

        if (star != null && star instanceof ListType) {
            star = ((ListType) star).toTupleType();
        }

        for (int i = 0, j = 0; i < argumentNumber; i++) {
            Node arg = args.get(i);
            Type aType = null;
            if (i < calledTypeSize) { // normal params
                aType = calledTypes.get(i);
            } else {
                if (hash != null && args.get(i) instanceof Name
                        && hash.containsKey(((Name) args.get(i)).id)) { // use keyword
                    aType = hash.get(((Name) args.get(i)).id);
                    hash.remove(((Name) args.get(i)).id);
                } else if (star != null && star instanceof TupleType
                        && j < ((TupleType) star).eltTypes.size()) { // use star
                    aType = ((TupleType) star).get(j++);
                }
            }
            
            if (aType == null) {
                if (i - nonDefaultNumber >= 0 && i - nonDefaultNumber < defaultTypeSize) { // use default
                    aType = defaultTypes.get(i - nonDefaultNumber);
                } else {
                    aType = Type.UNKNOWN;
                    if (call != null) {
                        Analyzer.self.putProblem(args.get(i),
                                "unable to bind argument:" + args.get(i));
                    }
                }
            }
            Binder.bind(funcTable, arg, aType, Binding.Kind.PARAMETER);
            fromType.add(aType);
        }
//  * * * ORIGINAL VERSION * * *
//        for (int i = 0, j = 0; i < argumentNumber; i++) {
//            Node arg = args.get(i);
//            Type aType;
//            if (i < calledTypeSize) {
//                aType = calledTypes.get(i);
//            } else if (i - nonDefaultNumber >= 0 && i - nonDefaultNumber < defaultTypeSize) {
//                aType = defaultTypes.get(i - nonDefaultNumber);
//            } else {
//                if (hash != null && args.get(i) instanceof Name
//                        && hash.containsKey(((Name) args.get(i)).id)) {
//                    aType = hash.get(((Name) args.get(i)).id);
//                    hash.remove(((Name) args.get(i)).id);
//                } else {
//                    if (star != null && star instanceof TupleType
//                            && j < ((TupleType) star).eltTypes.size()) {
//                        aType = ((TupleType) star).get(j++);
//                    } else {
//                        aType = Type.UNKNOWN;
//                        if (call != null) {
//                            Analyzer.self.putProblem(args.get(i),
//                                    "unable to bind argument:" + args.get(i));
//                        }
//                    }
//                }
//            }
//            Binder.bind(funcTable, arg, aType, Binding.Kind.PARAMETER);
//            fromType.add(aType);
//        }
        
        if (restKw != null) {
            if (hash != null && !hash.isEmpty()) {
                Type hashType = UnionType.newUnion(hash.values());
                Binder.bind(
                        funcTable,
                        restKw,
                        new DictType(Type.STR, hashType),
                        Binding.Kind.PARAMETER);
            } else {
                Binder.bind(funcTable,
                        restKw,
                        Type.UNKNOWN,
                        Binding.Kind.PARAMETER);
            }
        }

        if (rest != null) {
            if (calledTypes.size() > argumentNumber) {
                if (func.afterRest != null) {
                    int nAfter = func.afterRest.size();
                    for (int i = 0; i < nAfter; i++) {
                        Binder.bind(funcTable, func.afterRest.get(i),
                                calledTypes.get(calledTypes.size() - nAfter + i),
                                Binding.Kind.PARAMETER);
                    }
                    if (calledTypes.size() - nAfter > 0) {
                        Type restType = new TupleType(calledTypes.subList(argumentNumber, calledTypes.size() - nAfter));
                        Binder.bind(funcTable, rest, restType, Binding.Kind.PARAMETER);
                    }
                } else {
                    Type restType = new TupleType(calledTypes.subList(argumentNumber, calledTypes.size()));
                    Binder.bind(funcTable, rest, restType, Binding.Kind.PARAMETER);
                }
            } else {
                Binder.bind(funcTable,
                        rest,
                        Type.UNKNOWN,
                        Binding.Kind.PARAMETER);
            }
        }

        return fromType;
    }


    static void bindMethodAttrs(@NotNull FunType cl) {
        if (cl.table.parent != null) {
            Type cls = cl.table.parent.type;
            if (cls != null && cls instanceof ClassType) {
                addReadOnlyAttr(cl, "im_class", cls, CLASS);
                addReadOnlyAttr(cl, "__class__", cls, CLASS);
                addReadOnlyAttr(cl, "im_self", cls, ATTRIBUTE);
                addReadOnlyAttr(cl, "__self__", cls, ATTRIBUTE);
            }
        }
    }


    static void addReadOnlyAttr(@NotNull FunType fun,
                                String name,
                                @NotNull Type type,
                                Binding.Kind kind)
    {
        Node loc = Builtins.newDataModelUrl("the-standard-type-hierarchy");
        Binding b = new Binding(name, loc, type, kind);
        fun.table.update(name, b);
        b.markSynthetic();
        b.markStatic();
    }


    static boolean missingReturn(@NotNull Type toType) {
        boolean hasNone = false;
        boolean hasOther = false;

        if (toType instanceof UnionType) {
            for (Type t : ((UnionType) toType).types) {
                if (t == Type.NONE || t == Type.CONT) {
                    hasNone = true;
                } else {
                    hasOther = true;
                }
            }
        }

        return hasNone && hasOther;
    }

    /** It same as the node equals, but it checks the FunctionDef behind the Call **/
    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof Call)) {
            return false;
        } else {
            Call node = (Call) obj;
            String file = this.getFile();
            return (getStart() == node.getStart()
                    && getEnd() == node.getEnd()
                    && _.same(file, node.getFile()) // && getId() == node.getId()
                    && functionDefForStack == node.functionDefForStack);
        }
    }

    @NotNull
    @Override
    public String toString() {
        return "(call:" + func + ":" + args + ":" + getStart() + ")";
    }

}
