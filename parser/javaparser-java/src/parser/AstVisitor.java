// $Id$
// Created by Aron Barath, 2013

package parser;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;
import java.util.logging.Level;

import javax.persistence.EntityManager;
import javax.persistence.TypedQuery;

import org.eclipse.jdt.core.compiler.IProblem;
import org.eclipse.jdt.core.dom.ASTNode;
import org.eclipse.jdt.core.dom.Annotation;
import org.eclipse.jdt.core.dom.AnonymousClassDeclaration;
import org.eclipse.jdt.core.dom.ArrayAccess;
import org.eclipse.jdt.core.dom.Assignment;
import org.eclipse.jdt.core.dom.Block;
import org.eclipse.jdt.core.dom.ClassInstanceCreation;
import org.eclipse.jdt.core.dom.CompilationUnit;
import org.eclipse.jdt.core.dom.EnhancedForStatement;
import org.eclipse.jdt.core.dom.EnumConstantDeclaration;
import org.eclipse.jdt.core.dom.EnumDeclaration;
import org.eclipse.jdt.core.dom.Expression;
import org.eclipse.jdt.core.dom.FieldAccess;
import org.eclipse.jdt.core.dom.FieldDeclaration;
import org.eclipse.jdt.core.dom.IBinding;
import org.eclipse.jdt.core.dom.IExtendedModifier;
import org.eclipse.jdt.core.dom.IMethodBinding;
import org.eclipse.jdt.core.dom.ITypeBinding;
import org.eclipse.jdt.core.dom.IVariableBinding;
import org.eclipse.jdt.core.dom.ImportDeclaration;
import org.eclipse.jdt.core.dom.Javadoc;
import org.eclipse.jdt.core.dom.MethodDeclaration;
import org.eclipse.jdt.core.dom.MethodInvocation;
import org.eclipse.jdt.core.dom.Modifier;
import org.eclipse.jdt.core.dom.PackageDeclaration;
import org.eclipse.jdt.core.dom.PostfixExpression;
import org.eclipse.jdt.core.dom.PrefixExpression;
import org.eclipse.jdt.core.dom.QualifiedName;
import org.eclipse.jdt.core.dom.ReturnStatement;
import org.eclipse.jdt.core.dom.SimpleName;
import org.eclipse.jdt.core.dom.SimpleType;
import org.eclipse.jdt.core.dom.SingleVariableDeclaration;
import org.eclipse.jdt.core.dom.SuperConstructorInvocation;
import org.eclipse.jdt.core.dom.SuperMethodInvocation;
import org.eclipse.jdt.core.dom.TypeDeclaration;
import org.eclipse.jdt.core.dom.VariableDeclarationExpression;
import org.eclipse.jdt.core.dom.VariableDeclarationFragment;
import org.eclipse.jdt.core.dom.VariableDeclarationStatement;

import parser.JavaAstNode.AstType;
import parser.JavaAstNode.SymbolType;

public class AstVisitor extends org.eclipse.jdt.core.dom.ASTVisitor
{
	// http://help.eclipse.org/kepler/index.jsp?topic=%2Forg.eclipse.jdt.doc.isv%2Freference%2Fapi%2Forg%2Feclipse%2Fjdt%2Fcore%2Fdom%2FASTVisitor.html

	private CompilationUnit cu;
	private EntityManager em;
	private Project project;
	private File file; // source file
	private File classfile; // target (class) file
	private Frame frame = Frame.createFrame();
	private String currentPrefix = "*";
	private int classDeclDepth = 0;
	private Map<String, JavaFunction> functions = new HashMap<String, JavaFunction>(); // The functions meet by the visitor. Map mangled name to function. 
	private Map<String, JavaType> types = new HashMap<String, JavaType>(); // The types meet by the visitor. Map qualified name to type.
	private List<ClassInstanceCreation> anonymusClassInstantiations = new LinkedList<ClassInstanceCreation>();
	private int anonymusCounter = 1;
	private java.util.HashSet<String> implicitImports = new java.util.HashSet<String>();
	private java.util.HashSet<String> primitiveTypes = new java.util.HashSet<String>();
	private ProblemHandler problemHandler = null;
	private char[] content;
	private static final Logger log = Logger.getLogger(Parser.class.getName());

	public AstVisitor(EntityManager em, Project project, CompilationUnit cu, ProblemHandler problemHandler, String path, String filename, char[] src, long timestamp, String buildId, boolean createBuildAction) throws ParseException
	{
		this.em = em;
		this.cu = cu;
		this.project = project;
		this.problemHandler = problemHandler;
		this.content = src;

		createPrimitiveTypes();

		String classfilename = filename + ".class";

		if(classfilename.contains("."))
		{
			classfilename = filename.substring(0, filename.lastIndexOf('.')) + ".class";
		}
		
		File fparent = getDirectory(path, timestamp);

		if((file=getFileByName(path + "/" + filename))==null)
		{
			FileContent content = new FileContent();
			content.setContent(new String(src));
			content.setHash(Utils.computeContentHash(src));
			em.persist(content);

			file = new File();
			file.setPath(path + "/" + filename);
			file.setFilename(filename);
			file.setProject(project);
			file.setParent(fparent);
			file.setContent(content);
			file.setTimestamp(timestamp);
		}

		file.setType(File.JavaSource);
		em.persist(file);

		if((classfile=getFileByName(path + "/" + classfilename))==null)
		{
			classfile = new File();
			classfile.setPath(path + "/" + classfilename);
			classfile.setFilename(classfilename);
			classfile.setType(File.JavaClass);
			classfile.setProject(project);
			classfile.setParent(fparent);
			classfile.setContent(null);
			classfile.setTimestamp(timestamp);
			em.persist(classfile);
		}

		BuildAction ba = null;

		if(!buildId.isEmpty())
		{
			ba = em.find(BuildAction.class, Long.decode(buildId));
		}

		if(ba==null)
		{
			if(!buildId.isEmpty() && !createBuildAction)
			{
				throw new ParseException("BuildAction " + buildId + " is not exist.");
			}

			ba = new BuildAction();
			ba.setProject(project);
			em.persist(ba);

			BuildSource bs = new BuildSource();
			bs.setFile(file);
			bs.setAction(ba);
			em.persist(bs);

			BuildTarget bt = new BuildTarget();
			bt.setFile(classfile);
			bt.setAction(ba);
			em.persist(bt);
		}

		ba.setLabel(filename);
		ba.setType(BuildAction.Type.Compile);

		// store problems in database

		boolean no_errors = true;

		IProblem[] problems = cu.getProblems();

		if(problems!=null && problems.length>0)
		{
			for(IProblem problem : problems)
			{
				BuildLog log = new BuildLog();
				
				PositionInfo pi = new PositionInfo(cu, problem);

				log.setLoc_start_line(pi.getStartLine());
				log.setLoc_start_col(pi.getStartColumn());
				log.setLoc_end_line(pi.getEndLine());
				log.setLoc_end_col(pi.getEndColumn());

				log.setAction(ba);
				log.setFile(file);
				log.setLog_message(problem.toString());

				problemHandler.addJavacProblem(problem.isError(), pi.getStartLine() + ":" + pi.getStartColumn() + ": " + problem.toString());

				if(problem.isError())
				{
					no_errors = false;
					log.setType(BuildLog.Type.Error);
				}
				else
				if(problem.isWarning())
				{
					log.setType(BuildLog.Type.Warning);
				}
				else
				{
					log.setType(BuildLog.Type.Unknown);
				}

				em.persist(log);
			}
		}

		if(no_errors)
		{
			file.setParseStatus(File.PSFullyParsed);
		}
		else
		{
			file.setParseStatus(File.PSPartiallyParsed);
		}
	}

	private void error(ASTNode node, String message)
	{
		error(node, message, false);
	}

	private void error(ASTNode node, String message, boolean trace)
	{
		if(File.PSFullyParsed==file.getParseStatus())
		{
			file.setParseStatus(File.PSPartiallyParsed);
		}

		PositionInfo pi = new PositionInfo(cu, node);
		String completeMessage = pi.getStartLine() + ":" + pi.getStartColumn() + ": " + message;

		problemHandler.addParseError(completeMessage);

		if(trace)
		{
			log.log(Level.FINEST, completeMessage);
		}
	}

	private File getFileByName(String name)
	{
		TypedQuery<File> q = em.createQuery("SELECT f FROM File f WHERE f.path like \"" + name + "\"", File.class);
		return Utils.findFile(q, name);
	}

	private File getDirectory(String path, long timestamp)
	{
		List<String> dirs = Utils.splitPath(path);
		path = "/"; // replace path with the root
		File parent = getFileByName(path);

		if(parent==null)
		{
			// the first 'parent' is the root

			parent = new File();
			parent.setPath(path);
			parent.setFilename("");
			parent.setType(File.Directory);
			parent.setProject(project);
			parent.setParent(null);
			parent.setTimestamp(timestamp);
			em.persist(parent);
		}

		while(!dirs.isEmpty())
		{
			String name = dirs.get(0);
			path = path + (path.length()>1 ? "/" : "") + name;
			dirs.remove(0);

			File next_dir = getFileByName(path);

			if(next_dir==null)
			{
				next_dir = new File();
				next_dir.setPath(path);
				next_dir.setFilename(name);
				next_dir.setType(File.Directory);
				next_dir.setProject(project);
				next_dir.setParent(parent);
				next_dir.setTimestamp(timestamp);
				em.persist(next_dir);
			}

			parent = next_dir;
		}

		return parent;
	}

	private void trace(String s)
	{
		log.log(Level.FINEST, s);
	}

	private static int _calcNewNodeStart(int start, ASTNode node)
	{
		int nstart = node.getStartPosition() + node.getLength();

		if(nstart>start)
		{
			return nstart;
		}

		return start;
	}

	private static String whiteSpaces = "\r\n ";
	public int calcNodeOffset(ASTNode node, Javadoc doc, List mods)
	{
		int start = node.getStartPosition();

		if(null!=doc)
		{
			start = _calcNewNodeStart(start, doc);
		}

		if(null!=mods && !mods.isEmpty())
		{
			for(Object mod : mods)
			{
				if(mod instanceof ASTNode)
				{
					start = _calcNewNodeStart(start, (ASTNode)mod);
				}
			}
		}

		if(node.getStartPosition()!=start)
		{
			while(start<content.length && -1!=whiteSpaces.indexOf(content[start]))
			{
				++start;
			}
		}

		return start - node.getStartPosition();
	}

	private void createPrimitiveType(JavaAstNode node, String name)
	{
		TypedQuery<JavaType> ts = em.createQuery("SELECT t FROM JavaType t WHERE t.qualifiedName like \"" + name + "\"", JavaType.class);

		if(Utils.findType(ts, name)==null)
		{
			JavaType type = new JavaType();

			type.setAstNode(node);
			type.setName(name);
			type.setQualifiedName(name);
			type.setMangledNameHash(Utils.getNameHash(name));

			em.persist(type);
		}
	}

	private void createPrimitiveTypes()
	{
		JavaAstNode dummy_node = createOrGetDummyNode();

		primitiveTypes.add("void");
		primitiveTypes.add("byte");
		primitiveTypes.add("short");
		primitiveTypes.add("int");
		primitiveTypes.add("long");
		primitiveTypes.add("boolean");
		primitiveTypes.add("char");
		primitiveTypes.add("float");
		primitiveTypes.add("double");

		for(String primitiveType : primitiveTypes)
		{
			createPrimitiveType(dummy_node, primitiveType);
		}
	}

	private JavaAstNode createOrGetDummyNode()
	{
		final long dummyNodeId = permaAstId("dummy", "dummy");
		JavaAstNode node = em.find(JavaAstNode.class, new Long(dummyNodeId));

		if(null==node)
		{
			node = new JavaAstNode(dummyNodeId);
			em.persist(node);
		}

		return node;
	}

	private JavaType currentType()
	{
		return frame.getType();
	}

	private JavaFunction currentFunction()
	{
		return frame.getFunction();
	}

	private void pushVar(String name, JavaAstNode node)
	{
		trace("pushVar: " + name);

		frame.addVariable(name, node);
	}

	private JavaAstNode findVar(String name)
	{
		return frame.findVariable(name);
	}

	private int translateModifiers(int mods)
	{
		int m = 0;

		if((mods & Modifier.PUBLIC)!=0) { m |= JavaAstNode.Modifier.Public; }
		if((mods & Modifier.PROTECTED)!=0) { m |= JavaAstNode.Modifier.Protected; }
		if((mods & Modifier.PRIVATE)!=0) { m |= JavaAstNode.Modifier.Private; }
		if((mods & Modifier.STATIC)!=0) { m |= JavaAstNode.Modifier.Static; }
		if((mods & Modifier.ABSTRACT)!=0) { m |= JavaAstNode.Modifier.Abstract; }
		if((mods & Modifier.FINAL)!=0) { m |= JavaAstNode.Modifier.Final; }
		if((mods & Modifier.NATIVE)!=0) { m |= JavaAstNode.Modifier.Native; }
		if((mods & Modifier.SYNCHRONIZED)!=0) { m |= JavaAstNode.Modifier.Synchronized; }
		if((mods & Modifier.TRANSIENT)!=0) { m |= JavaAstNode.Modifier.Transient; }
		if((mods & Modifier.VOLATILE)!=0) { m |= JavaAstNode.Modifier.Volatile; }
		if((mods & Modifier.STRICTFP)!=0) { m |= JavaAstNode.Modifier.Strictfp; }

		return m;
	}

	private static long permaAstId(String val, String mangledName)
	{
		String id = val + ":not_in_file:" + mangledName;    
		return Utils.fnvHash(id);
	}

	private static long permaAstId(String val, String mangledName, File file, int start, int end)
	{
		String id = val + ":" + file.getId() + ":" + start + ":" + end + ":" + mangledName;
		return Utils.fnvHash(id);
	}

	private String getQualifedName(String var)
	{
		String name = frame.getQualifiedName();

		if(name.isEmpty())
		{
			return var;
		}
		else
		{
			return name + "." + var;
		}
	}

	private JavaAstNode createFakeAstNode(JavaType type, JavaType genericType)
	{
		String mangledName = type.getQualifiedName();
		String val = "type " + type.getName();
		final long id = permaAstId(val, mangledName, file, 0, 1);

		JavaAstNode n = em.find(JavaAstNode.class, new Long(id));

		if(null==n)
		{
			n = new JavaAstNode(id);

			n.setAstValue(val);
			n.setFile(file);
			n.setLoc_start_line(0);
			n.setLoc_start_col(0);
			n.setLoc_end_line(0);
			n.setLoc_end_col(1);
			n.setAstType(JavaAstNode.AstType.Definition);
			n.setSymbolType(JavaAstNode.SymbolType.Type);

			n.setMangledName(mangledName);
			n.setMangledNameHash(Utils.getNameHash(mangledName));

			em.persist(n);
		}

		return n;
	}

	private JavaAstNode createFakeAstNode(JavaFunction func, JavaFunction genericFunc)
	{
		String mangledName = func.getQualifiedName();
		String val = "func " + func.getName();
		final long id = permaAstId(val, mangledName, file, 0, 1);

		JavaAstNode n = em.find(JavaAstNode.class, new Long(id));

		if(null==n)
		{
			n = new JavaAstNode(id);

			n.setAstValue(val);
			n.setFile(file);
			n.setLoc_start_line(0);
			n.setLoc_start_col(0);
			n.setLoc_end_line(0);
			n.setLoc_end_col(1);
			n.setAstType(JavaAstNode.AstType.Definition);
			n.setSymbolType(JavaAstNode.SymbolType.Function);

			n.setMangledName(mangledName);
			n.setMangledNameHash(Utils.getNameHash(mangledName));

			em.persist(n);
		}

		return n;
	}

	private JavaAstNode createAstNode(ASTNode node, String val, JavaAstNode.AstType at, JavaAstNode.SymbolType st, String mangledName)
	{
		return createAstNode(node, val, at, st, mangledName, 0);
	}

	private JavaAstNode createAstNode(ASTNode node, String val, JavaAstNode.AstType at, JavaAstNode.SymbolType st, String mangledName, int offset)
	{
		PositionInfo pi = new PositionInfo(cu, node, offset);
		long id = permaAstId(val, mangledName, file, pi.getStart(), pi.getEnd());

		JavaAstNode n = em.find(JavaAstNode.class, new Long(id));

		if(null==n)
		{
			n = new JavaAstNode(id);

			n.setAstValue(val);
			n.setFile(file);
			n.setLoc_start_line(pi.getStartLine());
			n.setLoc_start_col(pi.getStartColumn());
			n.setLoc_end_line(pi.getEndLine());
			n.setLoc_end_col(pi.getEndColumn());
			n.setAstType(at);
			n.setSymbolType(st);

			em.persist(n);
		}
		else
		{
			em.refresh(n);
		}

		if(null!=mangledName)
		{
			n.setMangledName(mangledName);
			n.setMangledNameHash(Utils.getNameHash(mangledName));
		}

		return n;
	}

	private JavaAstNode createVariable(ASTNode node, JavaType type, String name, int mods, boolean local)
	{
		return createVariable(node, type, name, mods, local, null, null);
	}

	private JavaAstNode createVariable(ASTNode node, JavaType type, String name, int mods, boolean local, JavaFunction func)
	{
		return createVariable(node, type, name, mods, local, func, null);
	}

	private JavaAstNode createVariable(ASTNode node, JavaType type, String name, int mods, boolean local, JavaFunction func, Javadoc doc)
	{
		JavaAstNode n = createAstNode(node, "var " + name, JavaAstNode.AstType.Declaration, JavaAstNode.SymbolType.Variable, getQualifedName(name));

		if(null!=doc)
		{
			createJavadoc(n.getMangledNameHash(), doc);
		}

		if(func!=null)
		{
			registerParam(n, type, name, func);
		}
		else
		if(local)
		{
			registerLocal(n, type, name);
		}
		else
		{
			registerField(n, type, name, mods);
		}

		pushVar(name, n);

		return n;
	}
	
	private JavaFunction createFunction(MethodDeclaration node, JavaType retType, String name, String mangledName, int mods)
	{
		int node_offs = calcNodeOffset(node, node.getJavadoc(), node.modifiers());
		JavaAstNode new_node = createAstNode(node, "func " + name, JavaAstNode.AstType.Definition, JavaAstNode.SymbolType.Function, mangledName, node_offs);
		createJavadoc(new_node.getMangledNameHash(), node.getJavadoc());
		processAnnotations(new_node, node.modifiers());
		return registerFunction(node.resolveBinding(), new_node, retType, name, getQualifedName(name), mangledName, mods);
	}

	private JavaMember registerField(JavaAstNode node, JavaType fieldType, String name, int mods)
	{
		JavaType type = currentType();
		JavaMember mem = new JavaMember(type, node, fieldType, translateModifiers(mods));
		type.getFields().add(mem);
		mem.setName(name);
		em.persist(mem);

		JavaVariable var = new JavaVariable(fieldType, node, name, getQualifedName(name));
		var.setMangledNameHash(node.getMangledNameHash());
		em.persist(var);

		return mem;
	}

	private JavaVariable registerLocal(JavaAstNode n, JavaType type, String name)
	{
		JavaFunction func = currentFunction();
		JavaVariable var = new JavaVariable(type, n, name, getQualifedName(name));
		var.setMangledNameHash(n.getMangledNameHash());

		if(func!=null)
		{
			var.setLocalInFunc(func);
			func.getLocals().add(var);
		}
		em.persist(var);
		return var;
	}

	private JavaVariable registerParam(JavaAstNode n, JavaType type, String name, JavaFunction func)
	{
		JavaVariable var = new JavaVariable(type, n, name, getQualifedName(name));
		var.setMangledNameHash(n.getMangledNameHash());

		if(func!=null)
		{
			var.setParamInFunc(func);
			func.getParameters().add(var);
		}
		em.persist(var);
		return var;
	}

	private JavaFunction registerFunction(IMethodBinding binding, JavaAstNode node, JavaType retType, String name, String qualName, String mangledName, int mods)
	{
		JavaType type = currentType();
		JavaFunction func = functionLookUp(binding, mangledName, true);

		func.setType(type);
		func.setAstNode(node);
		func.setName(name);
		// func.setMangledNameHash(Utils.getNameHash(mangledName));
		func.setMangledName(mangledName);
		func.setQualifiedName(qualName);
		func.setReturnType(retType);
		func.setModifiers(translateModifiers(mods));

		type.getFunctions().add(func);

		return func;
	}

	private JavaDocComment createJavadoc(long mangledNameHash, Javadoc doc)
	{
		HtmlBuilder builder = new HtmlBuilder(doc);

		if(!builder.isEmpty())
		{
			JavaDocComment jd = new JavaDocComment(builder.getHash(), builder.getHtml(), mangledNameHash);
			em.persist(jd);
			return jd;
		}

		return null;
	}

	private void processAnnotations(JavaAstNode astnode, List mods)
	{
		if(null==mods) { return; }

		for(Object o : mods)
		{
			if(o instanceof IExtendedModifier)
			{
				IExtendedModifier iextmod = (IExtendedModifier)o;

				if(iextmod.isAnnotation())
				{
					Annotation ann = (Annotation)iextmod;

					JavaAnnotation ja = new JavaAnnotation(astnode, ann.getTypeName().getFullyQualifiedName());
					em.persist(ja);
				}
			}
		}
	}

	private JavaFunction loadOrCreateFunction(IMethodBinding binding, String mangledName)
	{
		// TypedQuery<JavaFunction> fs = em.createQuery("SELECT f FROM JavaFunction f WHERE f.mangledName like \"" + mangledName + "\"", JavaFunction.class);
		// JavaFunction func = Utils.findFunction(fs, mangledName);
		JavaFunction func = em.find(JavaFunction.class, new Long(Utils.getNameHash(mangledName)));

		if(null==func)
		{
			func = new JavaFunction();

			NameExtractor nex = NameExtractor.fromFunctionName(mangledName);

			func.setName(nex.getName());
			func.setMangledName(mangledName);
			func.setMangledNameHash(Utils.getNameHash(mangledName));
			func.setSignature(nex.getNameWithArgs());
			func.setQualifiedName(nex.getFullName());

			em.persist(func);

			if(null!=binding.getTypeArguments() && 0<mangledName.indexOf('<'))
			{
				for(ITypeBinding arg : binding.getTypeArguments())
				{
					func.getTypeParams().add(typeLookUp(arg));
				}
			}

			functions.put(mangledName, func);
		}

		return func;
	}

	private JavaFunction functionLookUp(IMethodBinding methbind, String mangledName)
	{
		return functionLookUp(methbind, mangledName, false);
	}

	private JavaFunction functionLookUp(IMethodBinding methbind, String mangledName, boolean eraseParams)
	{
		NameExtractor nex = NameExtractor.fromFunctionName(mangledName/*methbind.getQualifiedName()*/);
		String genQualName = nex.getFullName();
		String qualName = NameMangler.getMangledName(null, methbind.getMethodDeclaration());
//		String qualName = nex.removeGenericParams().getFullName();

		JavaFunction func;

		if(eraseParams)
		{
			func = functions.get(qualName);

			if(null==func)
			{
				func = loadOrCreateFunction(methbind, qualName);
			}
		}
		else
		{
			func = functions.get(genQualName);

			if(null==func)
			{
				JavaFunction genFunc = loadOrCreateFunction(methbind.getMethodDeclaration(), qualName);
				func = loadOrCreateFunction(methbind, genQualName);

				if(genFunc!=func)
				{
					genFunc.setIsGeneric(true);
					func.setGenericImpl(genFunc);
					createFakeAstNode(func, genFunc);
					func.setAstNode(genFunc.getAstNode());
				}
			}
		}

		updateFunctionAstNode(func);

		return func;
	}

	private JavaType loadOrCreateType(ITypeBinding binding, String qualName)
	{
		// TypedQuery<JavaType> ts = em.createQuery("SELECT t FROM JavaType t WHERE t.qualifiedName like \"" + qualName + "\"", JavaType.class);
		// JavaType type = Utils.findType(ts, qualName);
		JavaType type = em.find(JavaType.class, new Long(Utils.getNameHash(qualName)));

		if(null==type)
		{
			type = new JavaType();

			type.setName(NameExtractor.fromTypeName(qualName).getNameWithArgs());
			type.setQualifiedName(qualName);
			type.setMangledNameHash(Utils.getNameHash(qualName));
			type.setIsGeneric(false);

			em.persist(type);

			if(null!=binding.getTypeArguments() && 0<qualName.indexOf('<'))
			{
				for(ITypeBinding arg : binding.getTypeArguments())
				{
					type.getTypeParams().add(typeLookUp(arg));
				}
			}

			ITypeBinding superclass = binding.getSuperclass();
			if(superclass!=null)
			{
				type.setSuperClass(typeLookUp(superclass));
			}

			if(null!=binding.getInterfaces())
			{
				for(ITypeBinding iface : binding.getInterfaces())
				{
					type.getInterfaces().add(typeLookUp(iface));
				}
			}

			types.put(qualName, type);
		}

		return type;
	}

	private JavaType typeLookUp(ITypeBinding binding)
	{
		return typeLookUp(binding, false);
	}

	private JavaType typeLookUp(ITypeBinding binding, boolean eraseParams)
	{
		NameExtractor nex = NameExtractor.fromTypeName(binding.getQualifiedName());
		String genQualName = nex.getFullName();
		String qualName = nex.removeGenericParams().getQualName();

		JavaType type;

		if(eraseParams)
		{
			type = types.get(qualName);

			if(null==type)
			{
				type = loadOrCreateType(binding, qualName);
			}
		}
		else
		{
			type = types.get(genQualName);

			if(null==type)
			{
				JavaType genType = loadOrCreateType(binding, qualName);
				type = loadOrCreateType(binding, genQualName);

				if(genType!=type)
				{
					genType.setIsGeneric(true);
					type.setGenericImpl(genType);
					type.setAstNode(genType.getAstNode());
					createFakeAstNode(type, genType);
				}
			}
		}

		updateTypeAstNode(type);

		return type;
	}

	private void updateFunctionAstNode(JavaFunction func)
	{
		for(JavaFunction f : func.getGenericInst())
		{
			f.setAstNode(func.getAstNode());
		}
	}

	private void updateTypeAstNode(JavaType type)
	{
		for(JavaType t : type.getGenericInst())
		{
			t.setAstNode(type.getAstNode());
		}
	}

	private AstType getUsageMode(ASTNode node, String typeName)
	{
		ASTNode child = node;
		ASTNode pnode = node.getParent();

		while(pnode!=null)
		{
			if(pnode instanceof MethodInvocation)
			{
//				// objects are passed by reference, so they
//				// can be modified by the called function
//
//				if(primitiveTypes.contains(typeName))
//				{
//					// ... unless it is a primitive type, which passed by value
//					return AstType.Read;
//				}
//
//				return AstType.Write;
//
// TODO: due to an unknown reason it is changed to 'read'
//

				return AstType.Read;
			}
			else
			if(pnode instanceof PrefixExpression
				|| pnode instanceof PostfixExpression)
			{
				return AstType.Write;
			}
			else
			if(pnode instanceof ArrayAccess)
			{
				ArrayAccess access = (ArrayAccess)pnode;

				if(child==access.getIndex())
				{
					return AstType.Read;
				}
			}
			else
			if(pnode instanceof Assignment)
			{
				Assignment assignment = (Assignment)pnode;

				if(child==assignment.getLeftHandSide())
				{
					return AstType.Write;
				}
			}
			else
			if(pnode instanceof VariableDeclarationFragment)
			{
				VariableDeclarationFragment frag = (VariableDeclarationFragment)pnode;

				if(child==frag.getInitializer())
				{
					return AstType.Read;
				}
				else
				{
					return AstType.Write;
				}
			}

			child = pnode;
			pnode = pnode.getParent();
		}

		return AstType.Read;
	}

	public boolean visit(PackageDeclaration node)
	{
		String packageName = node.getName().getFullyQualifiedName();
		trace("Package: " + packageName);

		frame = Frame.createFrame(packageName);

		return true;
	}

	public boolean visit(ImportDeclaration node)
	{
		String name = node.getName().getFullyQualifiedName();

		JavaAstNode an = createAstNode(node, "import " + name, JavaAstNode.AstType.Declaration, JavaAstNode.SymbolType.Import, name);
		JavaImport im = new JavaImport(file, an, name, node.isStatic(), node.isOnDemand(), true);
		em.persist(im);

		// Import declaration is handled.
		return false;
	}

	private void handleImplicitImport(JavaAstNode an, String mangledName, boolean isFunction)
	{
		if(null==mangledName || mangledName.isEmpty())
		{
			return;
		}

		String qual;

		if(isFunction)
		{
			qual = NameExtractor.fromFunctionName(mangledName).removeGenericParams().getQualifier();
		}
		else
		{
			qual = NameExtractor.fromTypeName(mangledName).removeGenericParams().getQualName();
		}

		if(!qual.isEmpty()
			&& !qual.startsWith(currentPrefix)
			&& !primitiveTypes.contains(qual)
			&& !implicitImports.contains(qual))
		{
			implicitImports.add(qual);

			JavaImport im = new JavaImport(file, an, qual, false, false, false);
			em.persist(im);
		}
	}

	private boolean handleVisitTypeDeclaration(ASTNode node, String nodeName, ITypeBinding binding, boolean isEnum)
	{
		String className = binding.getQualifiedName();

		if(0==(classDeclDepth++))
		{
			currentPrefix = className;
		}

		String val;
		JavaAstNode.SymbolType st;
		Javadoc doc = null;
		List mods = null;
		int offs = 0;
		if(isEnum)
		{
			val = "enum " + className;
			st  = JavaAstNode.SymbolType.Enum;

			if(node instanceof EnumDeclaration)
			{
				EnumDeclaration enum_decl = (EnumDeclaration)node;

				offs = calcNodeOffset(node, enum_decl.getJavadoc(), enum_decl.modifiers());
				doc  = enum_decl.getJavadoc();
				mods = enum_decl.modifiers();
			}
		}
		else
		{
			val = "type " + className;
			st  = JavaAstNode.SymbolType.Type;

			if(node instanceof TypeDeclaration)
			{
				TypeDeclaration type_decl = (TypeDeclaration)node;

				offs = calcNodeOffset(node, type_decl.getJavadoc(), type_decl.modifiers());
				doc  = type_decl.getJavadoc();
				mods = type_decl.modifiers();
			}
		}

		JavaAstNode an = createAstNode(node, val, JavaAstNode.AstType.Definition, st, className, offs);

		if(null!=mods)
		{
			processAnnotations(an, mods);
		}

		if(null!=doc)
		{
			createJavadoc(an.getMangledNameHash(), doc);
		}

		JavaType type = typeLookUp(binding, true);
		type.setAstNode(an);
		type.setIsEnum(isEnum);

		frame = new Frame(frame, nodeName, type, null);

		return true;
	}

	public boolean visit(TypeDeclaration node)
	{
		ITypeBinding binding = node.resolveBinding();

		if(null==binding)
		{
			error(node, node.getName().toString());
			return false;
		}

		String className = binding.getQualifiedName();
		trace("Type declaration start: " + className);

		return handleVisitTypeDeclaration(node, node.getName().toString(), binding, false);
	}

	public void endVisit(TypeDeclaration node)
	{
		if((--classDeclDepth)==0)
		{
			currentPrefix = "*";
		}

		frame = frame.pop();
		trace("Type declaration end: " + node.getName());
	}

	public boolean visit(AnonymousClassDeclaration node)
	{
		String className = frame.getType().getQualifiedName() + "$" + (anonymusCounter++);
		trace("Anonymous type declaration start: " + className);

		try
		{
			if(!anonymusClassInstantiations.isEmpty())
			{
				ClassInstanceCreation inv = anonymusClassInstantiations.get(anonymusClassInstantiations.size()-1);
				anonymusClassInstantiations.remove(anonymusClassInstantiations.size()-1);
				IMethodBinding binding = inv.resolveConstructorBinding();
				handleMethodInvocation(inv, binding, className + NameMangler.getParameterTypes(frame, binding), Modifier.FINAL);
			}

			JavaAstNode an = createAstNode(node, "type " + className, JavaAstNode.AstType.Definition, JavaAstNode.SymbolType.Type, className);

			JavaType type = typeLookUp(node.resolveBinding());
			type.setAstNode(an);

			frame = new Frame(frame, className, type, null, true);
		}
		catch(Exception ex)
		{
			error(node, "Anonymous class " + className);
			return false;
		}

		return true;
	}

	public void endVisit(AnonymousClassDeclaration node)
	{
		trace("Anonymous type declaration end: " + frame.getQualifiedName());
		frame = frame.pop();
	}

	public boolean visit(EnumDeclaration node)
	{
		ITypeBinding binding = node.resolveBinding();
		if(null==binding)
		{
			error(node, node.getName().toString());
			return false;
		}
		String className = binding.getQualifiedName();
		trace("Enum declaration start: " + className);
		return handleVisitTypeDeclaration(node, node.getName().toString(), binding, true);
	}

	public void endVisit(EnumDeclaration node)
	{
		if((--classDeclDepth)==0)
		{
			currentPrefix = "*";
		}

		frame = frame.pop();
		trace("Enum declaration end: " + node.getName());
	}

	public boolean visit(EnumConstantDeclaration node)
	{
		assert(null!=currentType() && currentType().isEnum);

		String name = node.getName().getIdentifier();
		String mangledName = currentType().getQualifiedName() + "." + name;
		JavaAstNode an = createAstNode(node, "enumval " + name, AstType.Declaration, SymbolType.EnumConstant, mangledName);
		JavaEnumConstant ec = new JavaEnumConstant();
		ec.setAstNode(an);
		ec.setEnumType(currentType());
		ec.setMangledNameHash(an.getMangledNameHash());
		ec.setName(name);
		ec.setQualifiedName(mangledName);
		currentType().getEnumConstants().add(ec);
		em.persist(ec);

		//trace("EnumConstant declaration start: " + node.getName());

		return true;
	}

	public void endVisit(EnumConstantDeclaration node)
	{
		//trace("EnumConstant declaration end: " + node.getName());
	}

	public boolean visit(Block node)
	{
		frame = new Frame(frame, Frame.GenerateUnique, Frame.InheritType, Frame.InheritFunction);
		trace("Enter block in line " + cu.getLineNumber(node.getStartPosition()));
		return true;
	}

	public void endVisit(Block node)
	{
		trace("Leave block in line " + cu.getLineNumber(node.getStartPosition() + node.getLength() - 1));
		frame = frame.pop();
	}

	public boolean visit(MethodDeclaration node)
	{
		String name = node.getName().getIdentifier();
		String mangledName = NameMangler.getMangledName(frame, node);

		trace("visit MethodDeclaration " + mangledName);

		try
		{
			JavaType retType = (node.isConstructor() || node.getReturnType2()==null) ? null : typeLookUp(node.getReturnType2().resolveBinding());
			JavaFunction func = createFunction(node, retType, name, mangledName, node.getModifiers() | (node.isConstructor() ? Modifier.FINAL : 0));

			String frameName = NameExtractor.fromFunctionName(mangledName).getNameWithArgs();
			trace("  frame name: " + frameName);

			frame = new Frame(frame, frameName, Frame.InheritType, func);

			for(Object o : node.parameters())
			{
				if(o instanceof SingleVariableDeclaration)
				{
					SingleVariableDeclaration par = (SingleVariableDeclaration)o;

					try
					{
						JavaType type = typeLookUp(par.getType().resolveBinding());
						String varname = par.getName().getIdentifier();

						createVariable(par, type, varname, 0, false, func);

						trace("  par: " + varname);
					}
					catch(Exception ex)
					{
						error(par, "Parameter of " + name + ": " + par.getName().getIdentifier());
					}
				}
			}
		}
		catch(Exception ex)
		{
			error(node, name);
			return false;
		}

		return true;
	}

	public void endVisit(MethodDeclaration node)
	{
		trace("Leave function in line " + cu.getLineNumber(node.getStartPosition() + node.getLength() - 1));
		frame = frame.pop();
	}

	public void handleMethodInvocation(ASTNode node, IMethodBinding methbind)
	{
		if(null!=methbind)
		{
			try
			{
				handleMethodInvocation(node, methbind, NameMangler.getMangledName(frame, methbind), methbind.getModifiers());
			}
			catch(Exception ex)
			{
				error(node, "Exception caught when mangling name for method " + methbind.getName());
			}
		}
		else
		{
			error(node, "Missing method binding");
		}
	}

	public void handleMethodInvocation(ASTNode node, IMethodBinding methbind, String mangledName, int modifiers)
	{
		if(null!=methbind)
		{
			JavaFunction func = functionLookUp(methbind, mangledName);

			trace("Function call " + mangledName);

			JavaAstNode.AstType at = JavaAstNode.AstType.VirtualCall;

			if(0!=(modifiers & Modifier.FINAL) || 0!=(func.getModifiers() & JavaAstNode.Modifier.Final))
			{
				at = JavaAstNode.AstType.Usage;
			}

			JavaAstNode an = createAstNode(node, "call " + mangledName, at, JavaAstNode.SymbolType.Function, func.getMangledName());
			handleImplicitImport(an, mangledName, true);
		}
		else
		{
			error(node, "Missing method binding");
		}
	}

	public boolean visit(MethodInvocation node)
	{
		handleMethodInvocation(node, node.resolveMethodBinding());
		return true;
	}

	public boolean visit(SuperMethodInvocation node)
	{
		handleMethodInvocation(node, node.resolveMethodBinding());
		return true;
	}

	public boolean visit(SuperConstructorInvocation node)
	{
		handleMethodInvocation(node, node.resolveConstructorBinding());
		return true;
	}

	public boolean visit(ClassInstanceCreation node)
	{
		if(node.getAnonymousClassDeclaration()!=null)
		{
			anonymusClassInstantiations.add(node);
		}
		else
		{
			handleMethodInvocation(node, node.resolveConstructorBinding());
		}
		return true;
	}

	public boolean visit(SingleVariableDeclaration node)
	{
		SimpleName name = node.getName();
		trace("Declaration of '" + name + "' at line " + cu.getLineNumber(name.getStartPosition()));

		if(node.getParent() instanceof EnhancedForStatement)
		{
			ITypeBinding binding = node.getType().resolveBinding();

			if(null==binding)
			{
				error(node, name.getIdentifier());
				return false;
			}

			JavaType type = typeLookUp(binding);
			JavaAstNode an = createVariable(name, type, name.getIdentifier(), node.getModifiers(), true);

			if(null!=type && null!=type.getAstNode() && null!=type.getAstNode().getMangledName())
			{
				handleImplicitImport(an, type.getAstNode().getMangledName(), false);
			}

			if(null!=binding)
			{
				String id = binding.getName();
				String mangledName = binding.getQualifiedName();
				JavaAstNode an2 = createAstNode(node, "typeloc " + id, JavaAstNode.AstType.TypeLocation, JavaAstNode.SymbolType.Type, mangledName);
				an2.setMangledName(mangledName);
				an2.setMangledNameHash(Utils.getNameHash(mangledName));
	
				trace("Type location (svd) of '" + id + "' at line " + cu.getLineNumber(node.getStartPosition()));
			}

			// Successfully handled, return with false to stop visiting.
			return false;
		}

		// We don't know what is it...
		return true;
	}

	public boolean visit(VariableDeclarationFragment node)
	{
		SimpleName name = node.getName();
		trace("Declaration of '" + name + "' at line " + cu.getLineNumber(name.getStartPosition()));

		if(node.getParent() instanceof FieldDeclaration)
		{
			FieldDeclaration fd = (FieldDeclaration)node.getParent();
			ITypeBinding binding = fd.getType().resolveBinding();

			if(null==binding)
			{
				error(node, name.toString());
				return false;
			}

			JavaType type = typeLookUp(binding);

			JavaAstNode an = createVariable(node, type, node.getName().getIdentifier(), fd.getModifiers(), false, null, fd.getJavadoc());
			processAnnotations(an, fd.modifiers());
			handleImplicitImport(an, type.getQualifiedName(), false);
		}
		else
		if(node.getParent() instanceof VariableDeclarationExpression)
		{
			VariableDeclarationExpression vde = (VariableDeclarationExpression)node.getParent();
			ITypeBinding binding = vde.getType().resolveBinding();

			if(null==binding)
			{
				error(node, name.toString());
				return false;
			}

			JavaType type = typeLookUp(binding);

			JavaAstNode an = createVariable(node, type, node.getName().getIdentifier(), vde.getModifiers(), true);
			handleImplicitImport(an, type.getQualifiedName(), false);
		}
		else
		if(node.getParent() instanceof VariableDeclarationStatement)
		{
			VariableDeclarationStatement vds = (VariableDeclarationStatement)node.getParent();
			ITypeBinding binding = vds.getType().resolveBinding();

			if(null==binding)
			{
				error(node, name.toString());
				return false;
			}

			JavaType type = typeLookUp(binding);

			JavaAstNode an = createVariable(node, type, node.getName().getIdentifier(), vds.getModifiers(), true);
			handleImplicitImport(an, type.getQualifiedName(), false);
		}

		return true;
	}

	private boolean processQualifiedName(SimpleName name, QualifiedName qualName)
	{
		IBinding bind = qualName.resolveBinding();

		if(null==bind)
		{
			error(qualName, qualName.getName().toString());
		}
		else
		if(bind.getKind()==IBinding.VARIABLE)
		{
			// We got a variable binding.
			IVariableBinding vbind = (IVariableBinding)bind;

			if(vbind.getDeclaringClass()==null && ".length)I".equals(vbind.getKey()))
			{
				// workaround: there is a bug in the expression "<some_array>.length"
				return true;		
			}

			ITypeBinding tbind = qualName.resolveTypeBinding();

			if(null==tbind)
			{
				error(qualName, qualName.getName().toString());
				return false;
			}

			String typeName = tbind.getQualifiedName();

			// Mangled name is a little tricky...
			String mangledName = vbind.getDeclaringClass().getQualifiedName() + "." + vbind.getName();
			JavaAstNode an = createAstNode(name, "usage " + mangledName, getUsageMode(qualName, typeName), JavaAstNode.SymbolType.Variable, mangledName);
			an.setMangledName(mangledName);
			an.setMangledNameHash(Utils.getNameHash(mangledName));

			trace("Usage of '" + mangledName + "' at line " + cu.getLineNumber(name.getStartPosition()));

			// Node processed, don't go further.
			return false;
		}

		// If QualifiedName does not belong to a variable, then keep processing.
		return true;
	}

	private void processFieldAccess(SimpleName name, FieldAccess fieldAccess)
	{
		IVariableBinding fbind = fieldAccess.resolveFieldBinding();
		ITypeBinding tbind = fieldAccess.resolveTypeBinding();

		if(null==fbind || null==tbind)
		{
			error(fieldAccess, name.getIdentifier() + "/" + fieldAccess.getName());
		}
		else
		if(fbind.getDeclaringClass()!=null)
		{
			String id = name.getIdentifier();
			String mangledName = fbind.getDeclaringClass().getQualifiedName() + "." + fbind.getName();
			String typeName = tbind.getQualifiedName();

			JavaAstNode an = createAstNode(name, "usage " + id, getUsageMode(fieldAccess, typeName), JavaAstNode.SymbolType.Variable, mangledName);
			an.setMangledName(mangledName);
			an.setMangledNameHash(Utils.getNameHash(mangledName));

			trace("Usage of '" + id + "' at line " + cu.getLineNumber(name.getStartPosition()));
		}
	}

	public boolean visit(SimpleType type)
	{
		if(null!=type.getParent() && ((type.getParent() instanceof TypeDeclaration) || (type.getParent() instanceof EnumDeclaration)))
		{
			ITypeBinding binding = (ITypeBinding)type.resolveBinding();

			if(null==binding)
			{
				error(type, type.getName().toString());
				return false;
			}

			String mangledName = binding.getQualifiedName();
			String id = binding.getName();
			JavaAstNode an = createAstNode(type, "typeloc " + id, JavaAstNode.AstType.TypeLocation, JavaAstNode.SymbolType.Type, mangledName);
			an.setMangledName(mangledName);
			an.setMangledNameHash(Utils.getNameHash(mangledName));

			trace("Type location (st) of '" + id + "' at line " + cu.getLineNumber(type.getStartPosition()));

			return false;
		}

		return true;
	}

	public boolean visit(SimpleName node)
	{
		if(null!=node.getParent() && (node.getParent() instanceof FieldAccess) && ((FieldAccess)node.getParent()).getName()==node)
		{
			processFieldAccess(node, (FieldAccess)node.getParent());
			return false;
		}

		if(null!=node.getParent() && (node.getParent() instanceof QualifiedName) && ((QualifiedName)node.getParent()).getName()==node)
		{
			if(!processQualifiedName(node, (QualifiedName)node.getParent()))
			{
				return false;
			}
		}

		String id = node.getIdentifier();
		JavaAstNode var = findVar(id);

		boolean found = false;
		boolean isUsage = false;
		ASTNode pnode = node.getParent();

		while(!found && null!=pnode)
		{
			if(pnode instanceof MethodDeclaration)
			{
				found = true;
			}
			else
			if(pnode instanceof Expression || pnode instanceof ReturnStatement ||
				(pnode instanceof EnhancedForStatement && ((EnhancedForStatement)pnode).getExpression()==node))
			{
				found = true;
				isUsage = true;
			}
			else
			if(pnode instanceof TypeDeclaration && id.equals(((TypeDeclaration)pnode).getName().getIdentifier()))
			{
				found = true;
			}
			else
			if(pnode instanceof EnumDeclaration && id.equals(((EnumDeclaration)pnode).getName().getIdentifier()))
			{
				found = true;
			}
			else
			if(pnode instanceof VariableDeclarationFragment)
			{
				found   = true;
				isUsage = (null!=((VariableDeclarationFragment)pnode).getInitializer());
			}
			else
			if(pnode instanceof FieldDeclaration)
			{
				found = true;
			}

			pnode = pnode.getParent();
		}

		IBinding bind = node.resolveBinding();

		if(null!=bind)
		{
			if(null!=var && IBinding.VARIABLE==bind.getKind() && found && isUsage)
			{
				ITypeBinding tbind = node.resolveTypeBinding();

				if(null!=tbind)
				{
					String typeName = tbind.getQualifiedName();
					JavaAstNode an = createAstNode(node, "usage " + id, getUsageMode(node, typeName), JavaAstNode.SymbolType.Variable, var.getMangledName());
					an.setMangledName(var.getMangledName());
					an.setMangledNameHash(var.getMangledNameHash());
	
					trace("Usage of '" + typeName + "' at line " + cu.getLineNumber(node.getStartPosition()));
				}
				else
				{
					error(node, "Type of " + node.toString());
				}
			}
			else
			if(found && IBinding.TYPE==bind.getKind()
				&& !(node.getParent() instanceof TypeDeclaration) && !(node.getParent() instanceof EnumDeclaration))
			{
				ITypeBinding tbind = node.resolveTypeBinding();

				if(null!=tbind)
				{
					String mangledName = tbind.getQualifiedName();
					String nameWithArgs = NameExtractor.fromTypeName(mangledName).getNameWithArgs();
					JavaAstNode an = createAstNode(node, "typeloc " + nameWithArgs, JavaAstNode.AstType.TypeLocation, JavaAstNode.SymbolType.Type, mangledName);
					an.setMangledName(mangledName);
					an.setMangledNameHash(Utils.getNameHash(mangledName));
	
					trace("Type location (sn) of '" + mangledName + "' at line " + cu.getLineNumber(node.getStartPosition()));
				}
				else
				{
					error(node, "Type of " + node.toString());
				}
			}
		}
		else
		{
			error(node, node.toString());
		}

		return true;
	}
}
