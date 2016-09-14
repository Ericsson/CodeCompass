package org.yinwang.pysonar;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.LinkedHashSet;
import java.util.Set;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.yinwang.pysonar.ast.*;
import org.yinwang.pysonar.common.Extension;
import org.yinwang.pysonar.common.Modified;
import org.yinwang.pysonar.common.PPLogger;
import org.yinwang.pysonar.types.ModuleType;
import org.yinwang.pysonar.types.Type;
import org.yinwang.pysonar.types.UnionType;

public class Binding implements Comparable<Object> {

    @Modified
    public enum Kind {

        ATTRIBUTE(1), // attr accessed with "." on some other object
        CLASS(2), // class definition
        CONSTRUCTOR(3), // __init__ functions in classes
        FUNCTION(4), // plain function
        METHOD(5), // static or instance method
        MODULE(6), // file
        PARAMETER(7), // function param
        SCOPE(8), // top-level variable ("scope" means we assume it can have attrs)
        VARIABLE(9),
        MODULEREF(10);      // local variable

        private int val;

        Kind(int val) {
            this.val = val;
        }

        public int getValue() {
            return val;
        }
    }

    private int id;

    private boolean isStatic = false;         // static fields/methods
    private boolean isSynthetic = false;      // auto-generated bindings
    private boolean isBuiltin = false;        // not from a source file

    @NotNull
    public String name;     // unqualified name
    @NotNull
    public Node node;
    @NotNull
    public String qname;    // qualified name
    public Type type;       // inferred type
    public Kind kind;        // name usage context

    public Set<Node> refs = new LinkedHashSet<>(1);

    // fields from Def
    public int start = -1;
    public int end = -1;
    public int bodyStart = -1;
    public int bodyEnd = -1;

    public int bodyStartLine = -1;
    public int bodyStartColumn = -1;
    public int bodyEndLine = -1;
    public int bodyEndColumn = -1;

    @Nullable
    public String doc = null;

    @Nullable
    public String fileOrUrl;

    @Nullable
    public String mangledName = null;
    
    private boolean definitionBinding = false;

    public Binding(@NotNull String bindingName, @NotNull Node node,
            @NotNull Type type, @NotNull Kind kind, boolean register) {
        this.name = bindingName;
        this.qname = type.table.path;
        this.type = type;
        this.kind = kind;
        this.node = node;

        if (node instanceof Url) {
            String url = ((Url) node).url;
            if (url.startsWith("file://")) {
                fileOrUrl = url.substring("file://".length());
            } else {
                fileOrUrl = url;
            }
        } else {
            fileOrUrl = node.getFile();
            if (node instanceof Name) {
                name = ((Name) node).id;
            }
        }

        initLocationInfo(node);
        if (register) {
            Analyzer.self.registerBinding(this);
        }
    }

    @Modified
    public Binding(@NotNull String bindingName, @NotNull Node node,
            @NotNull Type type, @NotNull Kind kind) {
        this(bindingName, node, type, kind, true);
    }

    public static String formatQname(String qname) {
        if (qname == null) {
            return null;
        }
        String formQN = null;
        String separator = File.separator;

        String projectPath = Analyzer.self.projectDir;
        projectPath = projectPath.replace(separator, ".");

        String libPath = (String) Analyzer.self.libDir;
        if (libPath != null) {
            libPath = libPath.replace(separator, ".");
        }

        try {
            formQN = URLDecoder.decode(qname, "utf-8");
        } catch (UnsupportedEncodingException ex) {
            PPLogger.exception(ex);
        } catch (IllegalArgumentException ex) {
            PPLogger.exception(ex);
            PPLogger.warning("Invalid qname is: " + qname);
            return null;
        }
        formQN = new File(formQN).getPath();
        formQN = formQN.replace(" ", ".");

        if (formQN.startsWith(projectPath)) {
            formQN = formQN.replaceFirst(projectPath, "projectDir.");
        } else if (formQN.startsWith(libPath)) {
            formQN = formQN.replaceFirst(libPath, "libDir");
        } else {
            return null;
        }

        if (formQN.startsWith(".")) {
            formQN = formQN.substring(1);
        }
        return formQN;
    }

    public String getIdHash() {
        return node.idHash;
    }

    @Extension
    public void copyLocationInfo(Binding other) {
        qname = other.qname;    // qualified name

        start = other.start;
        end = other.end;
        bodyStart = other.bodyStart;
        bodyEnd = other.bodyEnd;

        bodyStartLine = other.bodyStartLine;
        bodyStartColumn = other.bodyStartColumn;
        bodyEndLine = other.bodyEndLine;
        bodyEndColumn = other.bodyEndColumn;
    }

    @Modified
    private void initLocationInfo(Node node) {
        start = node.getStart();
        end = node.getEnd();

        Node parent = node.parent;
        if ((parent instanceof FunctionDef && ((FunctionDef) parent).name == node)
                || (parent instanceof ClassDef && ((ClassDef) parent).name == node)) {
            bodyStart = parent.getStart();
            bodyEnd = parent.getEnd();

            bodyStartLine = parent.getLineStart();
            bodyStartColumn = parent.getOffset();
            bodyEndLine = parent.getLineEnd();
            bodyEndColumn = parent.getEndOffset();

        } else if (node instanceof Module) {
            name = ((Module) node).getName();
            start = 0;
            end = 0;
            bodyStart = node.getStart();
            bodyEnd = node.getEnd();

            bodyStartLine = node.getLineStart();
            bodyStartColumn = node.getOffset();
            bodyEndLine = node.getLineEnd();
            bodyEndColumn = node.getEndOffset();

        } else {
            bodyStart = node.getStart();
            bodyEnd = node.getEnd();

            bodyStartLine = node.getLineStart();
            bodyStartColumn = node.getOffset();
            bodyEndLine = node.getLineEnd();
            bodyEndColumn = node.getEndOffset();

        }
    }
//    private void initLocationInfo(Node node) {
//        start = node.getStart();
//        end = node.getEnd();
//
//        Node parent = node.parent;
//        if ((parent instanceof FunctionDef && ((FunctionDef) parent).name == node)
//                || (parent instanceof ClassDef && ((ClassDef) parent).name == node)) {
//            bodyStart = parent.getStart();
//            bodyEnd = parent.getEnd();
//
//            EnchancedNode eNode = Analyzer.self.createEnchancedNode(parent);
//            bodyStartLine = eNode.getLineStart();
//            bodyStartColumn = eNode.getOffset();
//            bodyEndLine = eNode.getLineEnd();
//            bodyEndColumn = eNode.getEndOffset();
//
//        } else if (node instanceof Module) {
//            name = ((Module) node).getName();
//            start = 0;
//            end = 0;
//            bodyStart = node.getStart();
//            bodyEnd = node.getEnd();
//
//            EnchancedNode eNode = Analyzer.self.createEnchancedNode(node);
//            bodyStartLine = eNode.getLineStart();
//            bodyStartColumn = eNode.getOffset();
//            bodyEndLine = eNode.getLineEnd();
//            bodyEndColumn = eNode.getEndOffset();
//
//        } else {
//            bodyStart = node.getStart();
//            bodyEnd = node.getEnd();
//
//            EnchancedNode eNode = Analyzer.self.createEnchancedNode(node);
//            if (eNode != null) {
//                bodyStartLine = eNode.getLineStart();
//                bodyStartColumn = eNode.getOffset();
//                bodyEndLine = eNode.getLineEnd();
//                bodyEndColumn = eNode.getEndOffset();
//            }
//        }
//    }

    public Str getDocstring() {
        Node parent = node.parent;
        if ((parent instanceof FunctionDef && ((FunctionDef) parent).name == node)
                || (parent instanceof ClassDef && ((ClassDef) parent).name == node)) {
            return parent.getDocString();
        } else {
            return node.getDocString();
        }
    }

    public void setQname(@NotNull String qname) {
        this.qname = qname;
    }

    public void addRef(Node node) {
        refs.add(node);
    }

    // merge one more type into the type
    // used by stateful assignments which we can't track down the control flow
    public void addType(Type t) {
        type = UnionType.union(type, t);
    }

    public void setType(Type type) {
        this.type = type;
    }

    public void setKind(Kind kind) {
        this.kind = kind;
    }

    public void markStatic() {
        isStatic = true;
    }

    public boolean isStatic() {
        return isStatic;
    }

    public void markSynthetic() {
        isSynthetic = true;
    }

    public boolean isSynthetic() {
        return isSynthetic;
    }

    public boolean isBuiltin() {
        return isBuiltin;
    }

    @NotNull
    public String getFirstFile() {
        Type bt = type;
        if (bt instanceof ModuleType) {
            String file = bt.asModuleType().file;
            return file != null ? file : "<built-in module>";
        }

        String file = getFile();
        if (file != null) {
            return file;
        }

        return "<built-in binding>";
    }

    @Nullable
    public String getFile() {
        return isURL() ? null : fileOrUrl;
    }

    @Nullable
    public String getURL() {
        return isURL() ? fileOrUrl : null;
    }

    public boolean isURL() {
        return fileOrUrl != null && fileOrUrl.startsWith("http://");
    }

    /**
     * Bindings can be sorted by their location for outlining purposes.
     */
    public int compareTo(@NotNull Object o) {
        return start - ((Binding) o).start;
    }

    @NotNull
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("(binding:");
        sb.append(":id=").append(id);
        sb.append(":kind=").append(kind);
        sb.append(":node=").append(node);
        sb.append(":type=").append(type);
        sb.append("location stats: [").append(start).append(end).
                append(bodyStartLine).append(bodyStartColumn).
                append(bodyEndLine).append(bodyEndColumn).append("]");
//        sb.append(":start=").append(start);
//        sb.append(":end=").append(end);
//        sb.append(":bodyStartLine=").append(bodyStartLine);
//        sb.append(":bodyStartColumn=").append(bodyStartColumn);
//        sb.append(":bodyEndLine=").append(bodyEndLine);
//        sb.append(":bodyEndColumn=").append(bodyEndColumn);
        sb.append(":qname=").append(qname);
        sb.append(":refs=");
        if (refs.size() > 10) {
            sb.append("[");
            sb.append(refs.iterator().next());
            sb.append(", ...(");
            sb.append(refs.size() - 1);
            sb.append(" more)]");
        } else {
            sb.append(refs);
        }
        return sb.toString();
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof Binding)) {
            return false;
        } else {
            Binding b = (Binding) obj;
            return (start == b.start
                    && end == b.end
                    && _.same(fileOrUrl, b.fileOrUrl));
        }
    }

    @Override
    public int hashCode() {
        return node.hashCode();
    }

    /**
     * @return the id
     */
    public int getId() {
        return id;
    }

    /**
     * @param id the id to set
     */
    public void setId(int id) {
        this.id = id;
    }
    
    public boolean isDefinitionBinding() {
        return definitionBinding;
    }

    public void markAsDefinitionBinding() {
        this.definitionBinding = true;
    }
}
