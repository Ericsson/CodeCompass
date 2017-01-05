package org.yinwang.pysonar.ast;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.yinwang.pysonar.Analyzer;
import org.yinwang.pysonar.Binding;
import org.yinwang.pysonar.State;
import org.yinwang.pysonar._;
import org.yinwang.pysonar.common.Modified;
import org.yinwang.pysonar.common.PPLogger;
import org.yinwang.pysonar.types.Type;
import org.yinwang.pysonar.types.UnionType;

/**
 * A Node is a junction in the program. Since there is no way to put different
 * things in the same segment of the same file, a node is uniquely identified by
 * a file, a start and end point.
 */
public abstract class Node implements java.io.Serializable, Comparable<Object> {

    public enum AstType {

        VARIABLE(1),
        FUNCTION(2),
        PARAMETER(3),
        CLASS(4),
        ATTRIBUTE(5),
        DECORATOR(6),
        MODULE(7),
        BINDINGLESS(8),
        UNKNOWN(9),
        UNRESOLVED(10),
        MODULEREF(11),
        CALL(12);
        private int val;

        AstType(int val) {
            this.val = val;
        }

        public int getValue() {
            return val;
        }
    }

    private final int ID;
    public String idHash = null;

    public String file;
    public int start;
    public int end;

    public String name;
    public Node parent = null;
    protected Node fakeParent = null;

    private String astType;

    private int lineStart;
    private int lineEnd;
    private int offset;
    private int endOffset;

    public boolean globalWrite = false;

    @Nullable
    public String builtinBindingId = null;

    @Nullable
    public String containerBinding = null;
    public AstType persistanceAstType;
    private boolean junk = false;

    @Modified
    public Node() {
        ID = Analyzer.self.getNodeId();
        this.astType = "UNKNOWN";
    }

    @Modified
    public Node(String file, int start, int end) {
        this();
        this.file = file;
        this.start = start;
        this.end = end;
        idHash = getFile() + ":" + getStart() + ":" + getEnd();
    }

    public Node(NameType type, String file, int start, int end,
            int lineStart, int offset, int lineEnd, int endOffset) {
        this(file, start, end);
        this.lineStart = lineStart;
        this.offset = offset;
        this.lineEnd = lineEnd;
        this.endOffset = endOffset;
    }

    public String getFullPath() {
        if (!file.startsWith("/")) {
            return _.makePathString(Analyzer.self.projectDir, getFile());
        } else {
            return getFile();
        }
    }

    public void setParent(Node parent) {
        this.parent = parent;
    }

    @NotNull
    public Node getAstRoot() {
        if (parent == null) {
            return this;
        }
        return parent.getAstRoot();
    }

    public int length() {
        return getEnd() - getStart();
    }

    public void addChildren(@Nullable Node... nodes) {
        if (nodes != null) {
            for (Node n : nodes) {
                if (n != null) {
                    n.setParent(this);
                }
            }
        }
    }

    public void addChildren(@Nullable Collection<? extends Node> nodes) {
        if (nodes != null) {
            for (Node n : nodes) {
                if (n != null) {
                    n.setParent(this);
                }
            }
        }
    }

    public void addFakeChild(@Nullable Block block) {
        if (block != null) {
            block.setFakeParent(this);
        }
    }

    public void addFakeChild(@Nullable Node... nodes) {
        if (nodes != null) {
            for (Node n : nodes) {
                if (n != null) {
                    n.setFakeParent(this);
                }
            }
        }
    }

    @Nullable
    public Str getDocString() {
        Node body = null;
        if (this instanceof FunctionDef) {
            body = ((FunctionDef) this).body;
        } else if (this instanceof ClassDef) {
            body = ((ClassDef) this).body;
        } else if (this instanceof Module) {
            body = ((Module) this).body;
        }

        if (body instanceof Block && ((Block) body).seq.size() >= 1) {
            Node firstExpr = ((Block) body).seq.get(0);
            if (firstExpr instanceof Expr) {
                Node docstrNode = ((Expr) firstExpr).value;
                if (docstrNode != null && docstrNode instanceof Str) {
                    return (Str) docstrNode;
                }
            }
        }
        return null;
    }

    @NotNull
    public static Type transformExpr(@NotNull Node n, State s) {
        return n.transform(s);
    }

    @NotNull
    protected abstract Type transform(State s);

    protected void addWarning(String msg) {
        Analyzer.self.putProblem(this, msg);
    }

    protected void addError(String msg) {
        Analyzer.self.putProblem(this, msg);
    }

    /**
     * Utility method to resolve every node in {@code nodes} and return the
     * union of their types. If {@code nodes} is empty or {@code null}, returns
     * a new {@link org.yinwang.pysonar.types.UnknownType}.
     */
    @NotNull
    protected Type resolveUnion(@NotNull Collection<? extends Node> nodes, State s) {
        Type result = Type.UNKNOWN;
        for (Node node : nodes) {
            Type nodeType = transformExpr(node, s);
            result = UnionType.union(result, nodeType);
        }
        return result;
    }

    /**
     * Resolves each element, also construct a result list.
     */
    @Nullable
    static protected List<Type> resolveList(@Nullable Collection<? extends Node> nodes, State s) {
        if (nodes == null) {
            return null;
        } else {
            List<Type> ret = new ArrayList<>();
            for (Node n : nodes) {
                ret.add(transformExpr(n, s));
            }
            return ret;
        }
    }

    // nodes are equal if they are from the same file and same starting point
    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof Node)) {
            return false;
        } else {
            Node node = (Node) obj;
            String file = this.getFile();
            return (getStart() == node.getStart()
                    && getEnd() == node.getEnd()
                    && _.same(file, node.getFile()) // && getId() == node.getId()
                    );
        }
    }

    @Override
    public int hashCode() {
        return (getFile() + ":" + getStart() + ":" + getEnd()).hashCode();
    }

    @Override
    public int compareTo(@NotNull Object o) {
        if (o instanceof Node) {
            return getStart() - ((Node) o).getStart();
        } else {
            return -1;
        }
    }

    public String toDisplay() {
        return "";
    }

    @NotNull
    @Override
    public String toString() {
        return "(node id:" + getId() + ":" + getFile() + ":" + getName() + ":"
                + getStart() + ":" + getEnd() + ":" + lineStart + ":"
                + offset + ":" + lineEnd + ":" + endOffset + ":" + astType + ")";
    }

    /**
     * @return the id
     */
    public int getId() {
        return ID;
    }

    public String getFile() {
        return file;
    }

    /**
     * @return the start
     */
    public int getStart() {
        return start;
    }

    /**
     * @return the end
     */
    public int getEnd() {
        return end;
    }

    /**
     * @return the name
     */
    public String getName() {
        return name;
    }

    /**
     * @return the astType
     */
    public String getAstType() {
        return astType;
    }

    /**
     * @return the lineStart
     */
    public int getLineStart() {
        return lineStart;
    }

    /**
     * @return the lineEnd
     */
    public int getLineEnd() {
        return lineEnd;
    }

    /**
     * @return the offset
     */
    public int getOffset() {
        return offset;
    }

    /**
     * @return the endOffset
     */
    public int getEndOffset() {
        return endOffset;
    }

    /**
     * @param astType the astType to set
     */
    public void setAstType(String astType) {
        this.astType = astType;
    }

    /**
     * @param lineStart the lineStart to set
     */
    public void setLineStart(int lineStart) {
        this.lineStart = lineStart;
    }

    /**
     * @param lineEnd the lineEnd to set
     */
    public void setLineEnd(int lineEnd) {
        this.lineEnd = lineEnd;
    }

    /**
     * @param offset the offset to set
     */
    public void setOffset(int offset) {
        this.offset = offset;
    }

    /**
     * @param endOffset the endOffset to set
     */
    public void setEndOffset(int endOffset) {
        this.endOffset = endOffset;
    }

    /**
     * @return the toHashString
     */
    public String getIdHash() {
        return idHash;
    }

    /**
     * @return the fakeParent
     */
    public Node getFakeParent() {
        return fakeParent;
    }

    /**
     * @param fakeParent the fakeParent to set
     */
    public void setFakeParent(Node fakeParent) {
        this.fakeParent = fakeParent;
    }

    public void markAsJunk() {
        junk = true;
    }

    public boolean isJunk() {
        return junk;
    }

    @Nullable
    public static Name getNameNode(Node n) {
        if (n instanceof Name) {
            return (Name) n;
        } else if (n instanceof Attribute) {
            return getNameNode(((Attribute) n).attr);
        } else if (n instanceof Call) {
            return getNameNode(((Call) n).func);
        }
        Analyzer.self.ds.getErrorLogger().addOtherError(
                "Node:getNameNode(): Unexpected Name node type in node: " + n);
        return null;
    }
    
    @Nullable
    public static ClassDef getContainerClass(Node n) {
        if (n instanceof ClassDef) {
            return (ClassDef) n;
        }
        if (n.getParent() != null) {
            return getContainerClass(n.getParent());
        } else {
            return null;
        }
    }

    @Nullable
    public static Call getContainerCall(Node n) {
        if (n instanceof Call) {
            return (Call) n;
        }
        if (n.parent != null) {
            return getContainerCall(n.parent);
        } else {
            return null;
        }
    }

    public static Integer getCalledParamNum(Node n) {
        if (n.parent instanceof Call) {
            return ((Call) n.parent).args.size();
        }

        if (n.parent == null) {
            return null;
        }

        if (n.parent instanceof Attribute || n.parent instanceof Subscript) {
            return Node.getCalledParamNum(n.parent);
        } else {
            return null;
        }
    }
    
    @Nullable
    public static FunctionDef getContainerMethod(Node n) {
        if (n == null) {
            return null;
        }

        if (n instanceof FunctionDef) {
            Binding functionBinding = Analyzer.self.ds.getBindingByBase(((FunctionDef) n).name);
            if (functionBinding != null) {
                return (functionBinding.kind == Binding.Kind.METHOD) 
                        ? (FunctionDef) n 
                        : Node.getContainerMethod(n.getParent());
            }
        } else {
            return Node.getContainerMethod(n.getParent());
        }

        return null;
    }
    
    public static boolean isFirstParamOfMethod(Node n) {
        FunctionDef fd = Node.getContainerMethod(n);
        
        if(fd != null && !fd.args.isEmpty()){
            return n.name.equals(fd.args.get(0).name);
        } else {
            return false;
        }
    }
    
    @Nullable
    public static FunctionDef getContainerConstructor(Node n) {
        if (n == null) {
            return null;
        }

        if (n instanceof FunctionDef) {
            Binding functionBinding = Analyzer.self.ds.getBindingByBase(((FunctionDef) n).name);
            if (functionBinding != null) {
                return (functionBinding.kind == Binding.Kind.CONSTRUCTOR) ? 
                        (FunctionDef) n : Node.getContainerConstructor(n.getParent());
            }
        } else {
            return Node.getContainerConstructor(n.getParent());
        }

        return null;
    }

    public static boolean isInConstructor(Node n) {
        return (Node.getContainerConstructor(n) != null);
    }

    /**
     * Test match only by name.
     */
    public static boolean isFirstParamOfConstructor(Node n) {
        FunctionDef ctr = Node.getContainerConstructor(n);
        
        if(ctr != null && !ctr.args.isEmpty()){
            return n.name.equals(ctr.args.get(0).name);
        } else {
            return false;
        }
    }

    public String getAbbreviatedQname() {
        if (file == null) {
            return "UNCATEGORIZED";
        }
        String tempFile = file;
        String projDir = Analyzer.self.projectDir;
        String libPath = Analyzer.self.libDir;
        String abvQname;

        try {
            tempFile = URLDecoder.decode(tempFile, "utf-8");
        } catch (UnsupportedEncodingException ex) {
            PPLogger.exception(ex);
        }
        tempFile = new File(tempFile).getPath();

        if (tempFile.startsWith(projDir)) {
            abvQname = tempFile.substring(projDir.length());
        } else if (tempFile.startsWith(libPath)) {
            abvQname = tempFile.substring(libPath.length());
        } else {
            abvQname = tempFile;
        }

        String sep = File.separator;
        abvQname = abvQname.replace(sep, ".");
        if(abvQname.startsWith(".")){
            abvQname = abvQname.substring(1, abvQname.length() - ".py".length());
        } else {
            abvQname = abvQname.substring(0, abvQname.length() - ".py".length());
        }
        return abvQname;
    }
    
    @Nullable
    public Node getParent() {
        return (parent != null) ? parent : fakeParent;
    }
}
