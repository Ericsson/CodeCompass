package org.yinwang.pysonar;

// ####################################### modified2
import cc.parser.PythonPersisterService;
import org.apache.thrift.TException;
// #######################################
import java.io.File;
import java.io.UnsupportedEncodingException;
import java.net.URL;
import java.net.URLDecoder;
import java.util.*;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.yinwang.pysonar.ast.Attribute;
import org.yinwang.pysonar.ast.Call;
import org.yinwang.pysonar.ast.Name;
import org.yinwang.pysonar.ast.Node;
import org.yinwang.pysonar.ast.Url;
import org.yinwang.pysonar.common.DataStorage;
import org.yinwang.pysonar.common.Extension;
import org.yinwang.pysonar.common.Modified;
import org.yinwang.pysonar.common.PPLogger;
import org.yinwang.pysonar.types.*;

public class Analyzer {

    // #######################################
    private PythonPersisterService.Iface persister;
    // #######################################

    public static String MODEL_LOCATION = "org/yinwang/pysonar/models";

    // global static instance of the analyzer itself
    public static Analyzer self;
    public String sid = _.newSessionId();
    public State moduleTable = new State(null, State.StateType.GLOBAL);
    public List<String> loadedFiles = new ArrayList<>();
    public State globaltable = new State(null, State.StateType.GLOBAL);
    public List<Binding> allBindings = new ArrayList<>();
    public Map<Node, List<Binding>> references = new LinkedHashMap<>();
    public Set<Name> resolved = new HashSet<>();
    public Set<Name> unresolved = new HashSet<>();
    public Map<String, List<Diagnostic>> semanticErrors = new HashMap<>();
    public Map<String, List<Diagnostic>> parseErrors = new HashMap<>();
    public String cwd = null;
    public int nCalled = 0;
    public boolean multilineFunType = false;
    public List<String> path = new ArrayList<>();
    private Set<FunType> uncalled = new HashSet<>();
    private Set<Object> callStack = new HashSet<>();
    private Set<Object> importStack = new HashSet<>();

    private AstCache astCache;
    public String cacheDir;
    public Set<String> failedToParse = new HashSet<>();
    public Stats stats = new Stats();
    public Builtins builtins;
    private Progress loadingProgress = null;

    public String libDir = null;
    public String projectDir;
    public String modelDir;
    public String suffix;

    public Map<String, Object> options;

    //------------------------------------------
    public DataStorage ds;
    private int nodeIdSeq = 1;
    private int initSeq = 1;

    public Analyzer(Map<String, Object> options, PythonPersisterService.Iface persister) {
        self = this;
        if (options != null) {
            this.options = options;
        } else {
            this.options = new HashMap<>();
        }
        ds = new DataStorage();
        this.persister = persister;
        this.stats.putInt("startTime", System.currentTimeMillis());
        this.suffix = ".py";
        this.builtins = new Builtins();
        this.builtins.init();
        addPythonPath();
        copyModels();
        createCacheDir();
        getAstCache();
    }

    public void terminateExecution() {
        try {
            persister.stop();
            Thread.dumpStack();
            System.exit(2);
        } catch (TException ex) {
            PPLogger.exception(ex);
        }
    }

    public void mementoBuiltinBindings() {
        for (Binding b : allBindings) {
            if (b.qname.startsWith("__builtin__.")) {
                b.node.builtinBindingId = b.qname.substring("__builtin__.".length());
            }
        }
    }

    private void putUnknownFunctionCall(Call call) {
        Name funcName = Node.getNameNode(call.func);
        if (funcName != null) {
            ds.addUnknownFuncCall(funcName, call);
        } else {
            PPLogger.warning("Analyzer.putUnknownFunctionCall: Unable to get function name: " + call);
        }
    }

    private void putUnknownAttribute(Attribute attribute) {
        Name attrName = Node.getNameNode(attribute);
        ds.addUnknownAttribute(attrName, attribute);
    }

    public void unknownTypeUsage(Attribute node) {
        if (node.parent != null) {
            if (node.parent instanceof Call) {
                putUnknownFunctionCall((Call) node.parent);
            } else {
                putUnknownAttribute(node);
            }
        } else {
            PPLogger.warning("Analyzer.unknownTypeUsage: node.parent is null:" + (Node) node);
        }
    }

    @Extension
    public int getNodeId() {
        return nodeIdSeq++;
    }
    
    @Extension
    public int getInitSeq() {
        return initSeq++;
    }

    public boolean hasOption(String option) {
        Object op = options.get(option);
        if (op != null && op.equals(true)) {
            return true;
        } else {
            return false;
        }
    }

    public void setOption(String option) {
        options.put(option, true);
    }

    // main entry to the analyzer
    public void analyze(String path) {
        String upath = _.unifyPath(path);
        File f = new File(upath);
        projectDir = f.isDirectory() ? f.getPath() : f.getParent();
        loadFileRecursive(upath);
    }

    public void analyze(Set<String> paths) {
        projectDir = _.commonPath(paths);
        _.msg("Calculated project dir: " + projectDir);

        libDir = (String) options.get("lib_path");
        if (libDir != null) {
            try {
                libDir = URLDecoder.decode(libDir, "utf-8");
                libDir = new File(libDir).getPath();
            } catch (UnsupportedEncodingException ex) {
                PPLogger.exception(ex);
            }
        } else {
            libDir = "Python_installation_dir_not_set";
        }

        for (String path : paths) {
            String upath = _.unifyPath(path);
            File f = new File(upath);
            loadFileRecursive(upath);
        }
    }

    public void setCWD(String cd) {
        if (cd != null) {
            cwd = _.unifyPath(cd);
        }
    }

    public void addPaths(@NotNull List<String> p) {
        for (String s : p) {
            addPath(s);
        }
    }

    public void addPath(String p) {
        path.add(_.unifyPath(p));
    }

    public void setPath(@NotNull List<String> path) {
        this.path = new ArrayList<>(path.size());
        addPaths(path);
    }

    private void addPythonPath() {
        String path = System.getenv("PYTHONPATH");
        if (path != null) {
            String[] segments = path.split(":");
            for (String p : segments) {
                addPath(p);
            }
        }
    }

    private void copyModels() {
        URL resource = Thread.currentThread().getContextClassLoader().getResource(MODEL_LOCATION);
        String dest = _.locateTmp("models");
        this.modelDir = dest;

        try {
            _.copyResourcesRecursively(resource, new File(dest));
            _.msg("copied models to: " + modelDir);
        } catch (Exception e) {
            _.die("Failed to copy models. Please check permissions of writing to: " + dest);
        }
        addPath(dest);
    }

    @NotNull
    public List<String> getLoadPath() {
        List<String> loadPath = new ArrayList<>();
        if (cwd != null) {
            loadPath.add(cwd);
        }
        if (projectDir != null && (new File(projectDir).isDirectory())) {
            loadPath.add(projectDir);
        }
        loadPath.addAll(path);

        ////
        try {
            for (String p : loadPath) {
                p = URLDecoder.decode(p, "utf-8");
                p = new File(p).getPath();
            }
        } catch (UnsupportedEncodingException | IllegalArgumentException ex) {
            PPLogger.exception(ex);
        }
        ////
        return loadPath;
    }

    public boolean inStack(Object f) {
        return callStack.contains(f);
    }

    public void pushStack(Object f) {
        callStack.add(f);
    }

    public void popStack(Object f) {
        callStack.remove(f);
    }

    public boolean inImportStack(Object f) {
        return importStack.contains(f);
    }

    public void pushImportStack(Object f) {
        importStack.add(f);
    }

    public void popImportStack(Object f) {
        importStack.remove(f);
    }

    @NotNull
    public List<Binding> getAllBindings() {
        return allBindings;
    }

    @Nullable
    ModuleType getCachedModule(String file) {
        Type t = moduleTable.lookupType(_.moduleQname(file));
        if (t == null) {
            return null;
        } else if (t instanceof UnionType) {
            for (Type tt : ((UnionType) t).types) {
                if (tt instanceof ModuleType) {
                    return (ModuleType) tt;
                }
            }
            return null;
        } else if (t instanceof ModuleType) {
            return (ModuleType) t;
        } else {
            return null;
        }
    }

    public List<Diagnostic> getDiagnosticsForFile(String file) {
        List<Diagnostic> errs = semanticErrors.get(file);
        if (errs != null) {
            return errs;
        }
        return new ArrayList<>();
    }

    public void putRef(@NotNull Node node, @NotNull Collection<Binding> bs) {
        if (!(node instanceof Url)) {
            List<Binding> bindings = references.get(node);
            if (bindings == null) {
                bindings = new ArrayList<>(1);
                references.put(node, bindings);
            }
            for (Binding b : bs) {
                if (!bindings.contains(b)) {
                    bindings.add(b);
                }
                b.addRef(node);
            }
        }
    }

    public void putRef(@NotNull Node node, @NotNull Binding b) {
        List<Binding> bs = new ArrayList<>();
        bs.add(b);
        putRef(node, bs);
    }

    @NotNull
    public Map<Node, List<Binding>> getReferences() {
        return references;
    }

    public void putProblem(@NotNull Node loc, String msg) {
        String file = loc.getFile();
        if (file != null) {
            addFileErr(file, loc.getStart(), loc.getEnd(), msg);
        }
    }

    // for situations without a Node
    public void putProblem(@Nullable String file, int begin, int end, String msg) {
        if (file != null) {
            addFileErr(file, begin, end, msg);
        }
    }

    void addFileErr(String file, int begin, int end, String msg) {
        Diagnostic d = new Diagnostic(file, Diagnostic.Category.ERROR, begin, end, msg);
        getFileErrs(file, semanticErrors).add(d);
    }

    List<Diagnostic> getFileErrs(String file, @NotNull Map<String, List<Diagnostic>> map) {
        List<Diagnostic> msgs = map.get(file);
        if (msgs == null) {
            msgs = new ArrayList<>();
            map.put(file, msgs);
        }
        return msgs;
    }

    @Nullable
    public Type loadFile(String path) {
        path = _.unifyPath(path);
        File f = new File(path);

        if (!f.canRead()) {
            return null;
        }
        Type module = getCachedModule(path);
        if (module != null) {
            return module;
        }

        // detect circular import
        if (Analyzer.self.inImportStack(path)) {
            return null;
        }

        // set new CWD and save the old one on stack
        String oldcwd = cwd;
        setCWD(f.getParent());

        Analyzer.self.pushImportStack(path);
        Type type = parseAndResolve(path);
        Analyzer.self.popImportStack(path);

        // restore old CWD
        setCWD(oldcwd);
        return type;
    }

    @Nullable
    private Type parseAndResolve(String file) {
        loadingProgress.tick();

        try {
            Node ast = getAstForFile(file);

            if (ast == null) {
                failedToParse.add(file);
                return null;
            } else {
                Type type = Node.transformExpr(ast, moduleTable);
                loadedFiles.add(file);
                return type;
            }
        } catch (OutOfMemoryError e) {
            if (astCache != null) {
                astCache.clear();
            }
            System.gc();
            return null;
        }
    }

    private void createCacheDir() {
        cacheDir = _.locateTmp("ast_cache");
        File f = new File(cacheDir);
        _.msg("AST cache is at: " + cacheDir);

        if (!f.exists()) {
            if (!f.mkdirs()) {
                _.die("Failed to create tmp directory: " + cacheDir
                        + ".Please check permissions");
            }
        }
    }

    private AstCache getAstCache() {
        if (astCache == null) {
            astCache = AstCache.get();
        }
        return astCache;
    }

    /**
     * Returns the syntax tree for {@code file}.
     * <p>
     */
    @Nullable
    public Node getAstForFile(String file) {
        return getAstCache().getAST(file);
    }

    @Nullable
    public ModuleType getBuiltinModule(@NotNull String qname) {
        return builtins.get(qname);
    }

    @Nullable
    public String makeQname(@NotNull List<Name> names) {
        if (names.isEmpty()) {
            return "";
        }

        String ret = "";

        for (int i = 0; i < names.size() - 1; i++) {
            ret += names.get(i).id + ".";
        }

        ret += names.get(names.size() - 1).id;
        return ret;
    }

    public String locateLocalModule(String headName, int level) {
        String path = cwd;
        for (int i = 1; i < level; ++i) {
            File f = new File(path);
            path = f.getParent();
        }
        List<String> paths = new ArrayList<>();
        paths.add(path);

        return locateModule(paths, headName);
    }

    public String locateLocalModule(String headName) {
        return locateModule(getLoadPath(), headName);
    }

    /**
     * Find the path that contains modname. Used to find the starting point of
     * locating a qname.
     *
     * @param headName first module name segment
     */
    public String locateModule(List<String> paths, String headName) {
        for (String p : paths) {
            File startDir = new File(p, headName);
            File initFile = new File(_.joinPath(startDir, "__init__.py").getPath());

            if (initFile.exists()) {
                return p;
            }

            File startFile = new File(startDir + suffix);
            if (startFile.exists()) {
                return p;
            }
        }

        return null;
    }

    // Should be rename to locateDepsModule
    // Note: If lib is added, it should be the last deps
    public String locateLibModule(String headName) {
        List<String> DepsPathes = (List<String>) options.get("deps");

        for (String DepPath : DepsPathes) {
            File startDir = new File(DepPath, headName);
            File initFile = new File(_.joinPath(startDir, "__init__.py").getPath());

            if (initFile.exists()) {
                return DepPath;
            }

            File startFile = new File(startDir + suffix);
            if (startFile.exists()) {
                return DepPath;
            }
        }

        return null;
    }

    public Type loadModule(@NotNull List<Name> name, int level, @NotNull State state) {
        if (level == 0) {
            return loadModule(name, state);
        } else {
            return loadLocalModule(name, level, state);
        }
    }

    /**
     *
     * @param name
     * @param state
     * @return
     */
    @Nullable
    public Type loadModule(@NotNull List<Name> name, @NotNull State state) {
        if (name.isEmpty()) {
            return null;
        }

        Type builtin = loadBuiltinModule(name, state);
        Type lib = loadLibModule(name, state);

        if (builtin != null && lib != null) {
//            lib.table.merge(builtin.table);
            lib.table.mergePriorOther(builtin.table);
            return builtin;
        } else if (builtin != null) {
            return builtin;
        } else if (lib != null) {
            return lib;
        }

        return loadLocalModule(name, state);
    }

    private Type loadBuiltinModule(@NotNull List<Name> name, @NotNull State state) {
        String qname = makeQname(name);
        Type mt = getBuiltinModule(qname);
        if (mt != null) {
            state.insert(name.get(0).id,
                    new Url(Builtins.LIBRARY_URL + mt.table.path + ".html"),
                    mt, Binding.Kind.SCOPE);
            return mt;
        }

        return null;
    }

    private Type loadLibModule(@NotNull List<Name> name, @NotNull State state) {
        return loadModuleByStartPath(locateLibModule(name.get(0).id), name, state);
    }

    private Type loadLocalModule(@NotNull List<Name> name, @NotNull State state) {
        return loadModuleByStartPath(locateLocalModule(name.get(0).id), name, state);
    }

    private Type loadLocalModule(@NotNull List<Name> name, int level, @NotNull State state) {
        return loadModuleByStartPath(locateLocalModule(name.get(0).id, level), name, state);
    }

    private Type loadModuleByStartPath(String startPath, @NotNull List<Name> name, @NotNull State state) {
        Type prev = null;

        if (startPath == null) {
            return null;
        }

        File path = new File(startPath);

        for (int i = 0; i < name.size(); i++) {
            path = new File(path, name.get(i).id);
            File initFile = new File(_.joinPath(path, "__init__.py").getPath());

            if (initFile.exists()) {
                Type mod = loadFile(initFile.getPath());
                if (mod == null) {
                    return null;
                }
                // Module binding should be the only one element in the set
                Binding moduleBinding = moduleTable.table.get(
                        ((ModuleType) mod).qname).iterator().next();

                if (prev != null) {
                    prev.table.insert(name.get(i).id, moduleBinding.node, mod, Binding.Kind.VARIABLE);
                } else {
                    state.insert(name.get(i).id, moduleBinding.node, mod, Binding.Kind.VARIABLE);
                }
                putRef(name.get(i), moduleBinding);
                prev = mod;

            } else if (i == name.size() - 1) {
                File startFile = new File(path + suffix);
                if (startFile.exists()) {
                    Type mod = loadFile(startFile.getPath());
                    if (mod == null) {
                        return null;
                    }
                    // Module binding should be the only one element in the set
                    Binding moduleBinding = moduleTable.table.get(
                            ((ModuleType) mod).qname).iterator().next();

                    if (prev != null) {
                        prev.table.insert(name.get(i).id, moduleBinding.node, mod, Binding.Kind.VARIABLE);
                    } else {
                        state.insert(name.get(i).id, moduleBinding.node, mod, Binding.Kind.VARIABLE);
                    }
                    putRef(name.get(i), moduleBinding);
                    prev = mod;
                } else {
                    return null;
                }
            }
        }
        return prev;
    }

    /**
     * Load all Python source files recursively if the given fullname is a
     * directory; otherwise just load a file. Looks at file extension to
     * determine whether to load a given file.
     */
    public void loadFileRecursive(String fullname) {
        int count = countFileRecursive(fullname);
        if (loadingProgress == null) {
            loadingProgress = new Progress(count, 50);
        }

        File file_or_dir = new File(fullname);

        if (file_or_dir.isDirectory()) {
            for (File file : file_or_dir.listFiles()) {
                loadFileRecursive(file.getPath());
            }
        } else {
            if (file_or_dir.getPath().endsWith(suffix)) {
                loadFile(file_or_dir.getPath());
            }
        }
    }

    // count number of .py files
    public int countFileRecursive(String fullname) {
        File file_or_dir = new File(fullname);
        int sum = 0;

        if (file_or_dir.isDirectory()) {
            for (File file : file_or_dir.listFiles()) {
                sum += countFileRecursive(file.getPath());
            }
        } else {
            if (file_or_dir.getPath().endsWith(suffix)) {
                sum += 1;
            }
        }
        return sum;
    }

    public void nullAll() {
        moduleTable = null;
        loadedFiles = null;
        globaltable = null;
        resolved = null;
        unresolved = null;
        semanticErrors = null;
        parseErrors = null;
        path = null;
        uncalled = null;
        callStack = null;
        importStack = null;
    }

    @Modified
    public void finish() {
        _.msg("\nFinished loading files. " + nCalled + " functions were called.");
        _.msg("Analyzing uncalled functions");
        applyUncalled();
//        mementoBuiltinBindings();

        _.msg(getAnalysisSummary());
        nullAll();
    }

    public void close() {
        astCache.close();
    }

    public void addUncalled(@NotNull FunType cl) {
        if (!cl.func.called) {
            uncalled.add(cl);
        }
    }

    public void removeUncalled(FunType f) {
        uncalled.remove(f);
    }

    private LinkedList<FunType> priorizeConstructors(Set<FunType> uncalled) {
        LinkedList<FunType> inits = new LinkedList<>();
        LinkedList<FunType> others = new LinkedList<>();

        for (FunType f : uncalled) {
            if (f.isConstructor()) {
                inits.add(f);
            } else {
                others.add(f);
            }
        }

        Collections.sort(inits, new Comparator<FunType>() {
            public int compare(FunType o1, FunType o2) {
                if (o1.getInitSeq() == o2.getInitSeq()) {
                    return 0;
                }
                return o1.getInitSeq() < o2.getInitSeq() ? -1 : 1;
            }
        });

        for (FunType o : others) {
            inits.add(o);
        }
        return inits;
    }

    public void applyUncalled() {
        Progress progress = new Progress(uncalled.size(), 50);

        while (!uncalled.isEmpty()) {
            List<FunType> uncalledDup = priorizeConstructors(uncalled);

            for (FunType cl : uncalledDup) {
                progress.tick();
                Call.apply(cl, null, null, null, null, null);
            }
        }
    }

    @NotNull
    public String getAnalysisSummary() {
        StringBuilder sb = new StringBuilder();
        sb.append("\n" + _.banner("analysis summary"));

        String duration = _.formatTime(System.currentTimeMillis() - stats.getInt("startTime"));
        sb.append("\n- total time: " + duration);
        sb.append("\n- modules loaded: " + loadedFiles.size());
        sb.append("\n- semantic problems: " + semanticErrors.size());
        sb.append("\n- failed to parse: " + failedToParse.size());

        // calculate number of defs, refs, xrefs
        int nDef = 0, nXRef = 0;
        for (Binding b : getAllBindings()) {
            nDef += 1;
            nXRef += b.refs.size();
        }

        sb.append("\n- number of definitions: " + nDef);
        sb.append("\n- number of cross references: " + nXRef);
        sb.append("\n- number of references: " + getReferences().size());

        long resolved = Analyzer.self.resolved.size();
        long unresolved = Analyzer.self.unresolved.size();
        sb.append("\n- resolved names: " + resolved);
        sb.append("\n- unresolved names: " + unresolved);
        sb.append("\n- name resolve rate: " + _.percent(resolved, resolved + unresolved));
        sb.append("\n" + _.getGCStats());

        return sb.toString();
    }

    @NotNull
    public List<String> getLoadedFiles() {
        List<String> files = new ArrayList<>();
        for (String file : loadedFiles) {
            if (file.endsWith(suffix)) {
                files.add(file);
            }
        }
        return files;
    }

    @Modified
    public void registerBinding(@NotNull Binding b) {
        allBindings.add(b);
        ds.addBaseBinding(b);
        b.setId(allBindings.size());
    }

    @NotNull
    @Override
    public String toString() {
        return "(analyzer:"
                + "[" + allBindings.size() + " bindings] "
                + "[" + references.size() + " refs] "
                + "[" + loadedFiles.size() + " files] "
                + ")";
    }
}
