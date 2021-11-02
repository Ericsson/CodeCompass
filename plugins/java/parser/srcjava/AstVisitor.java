package parser.srcjava;

import model.*;
import model.enums.AstType;
import model.enums.MemberTypeKind;
import model.enums.SymbolType;
import model.enums.Visibility;
import org.eclipse.jdt.core.Flags;
import org.eclipse.jdt.core.dom.*;
import org.eclipse.jdt.core.dom.Expression;

import javax.persistence.EntityManager;
import javax.persistence.criteria.*;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;
import java.util.function.Consumer;

public class AstVisitor extends ASTVisitor {

  private final CompilationUnit cu;
  private final EntityManager em;
  private final CriteriaBuilder cb;
  private final long fileId;

  public AstVisitor(CompilationUnit cu, EntityManager em, long fileId) {
    this.cu = cu;
    this.em = em;
    this.cb = em.getCriteriaBuilder();
    this.fileId = fileId;
  }

  @Override
  public boolean visit(AnnotationTypeDeclaration node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(AnnotationTypeMemberDeclaration node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(AnonymousClassDeclaration node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ArrayAccess node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ArrayCreation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ArrayInitializer node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ArrayType node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(AssertStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(Assignment node) {
    // System.out.println("ASSIGNMENT");
    // System.out.println(node);
    // System.out.println(node.getLeftHandSide().resolveTypeBinding());
    return super.visit(node);
  }

  @Override
  public boolean visit(Block node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(CastExpression node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(CatchClause node) {
    SingleVariableDeclaration variableDeclaration = node.getException();

    visitLocalVarDeclaration(variableDeclaration);

    return super.visit(node);
  }

  @Override
  public boolean visit(ClassInstanceCreation node) {
    JavaConstructor javaConstructor = new JavaConstructor();
    IMethodBinding methodDeclBinding =
      node.resolveConstructorBinding().getMethodDeclaration();
    Type type = node.getType();
    String qualifiedName = type.resolveBinding().getQualifiedName();
    String entityHashStr = String.join(
      " ", qualifiedName, methodDeclBinding.toString());
    int modifiers = methodDeclBinding.getModifiers();
    int classHash = qualifiedName.hashCode();
    int entityHash = entityHashStr.hashCode();

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.CONSTRUCTOR, AstType.USAGE, entityHash);

    persistJavaMemberType(
      classHash, classHash, MemberTypeKind.CONSTRUCTOR, modifiers, javaAstNode);

    setJavaEntityFields(
      javaConstructor, javaAstNode.getId(),
      entityHash, type.toString(), qualifiedName
    );

    persistRow(javaConstructor);

    return super.visit(node);
  }

  @Override
  public boolean visit(ConstructorInvocation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(EmptyStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(EnhancedForStatement node) {
    SingleVariableDeclaration variableDeclaration = node.getParameter();

    visitLocalVarDeclaration(variableDeclaration);

    return super.visit(node);
  }

  @Override
  public boolean visit(EnumDeclaration node) {
    JavaEnum javaEnum = new JavaEnum();
    SimpleName simpleName = node.getName();
    String declaringClassName = node.resolveBinding().getQualifiedName();
    int modifiers = node.getModifiers();
    int entityHash = declaringClassName.hashCode();

    // Persist Enum constants
    List<?> enumConstants = node.enumConstants();
    for (int i = 0; i < enumConstants.size(); i++) {
      visitEnumConstantDeclaration(
        javaEnum, (EnumConstantDeclaration) enumConstants.get(i), i);
    }

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.ENUM, AstType.DEFINITION, entityHash);

    persistJavaMemberType(
      entityHash, entityHash, MemberTypeKind.ENUM, modifiers, javaAstNode);

    setJavaEntityFields(
      javaEnum, javaAstNode.getId(), entityHash,
      simpleName.toString(), declaringClassName
    );

    persistInterfaceImplementations(node.superInterfaceTypes(), entityHash);

    persistRow(javaEnum);

    return super.visit(node);
  }

  @Override
  public boolean visit(ExportsDirective node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ExpressionMethodReference node) {
    // System.out.println("))))))))))))))))))))))))))))))))))))))))))))))))))))))");
    // System.out.println(node.getExpression().resolveTypeBinding().getDeclaringMethod());
    return super.visit(node);
  }

  @Override
  public boolean visit(ExpressionStatement node) {
    // System.out.println(node.getExpression().resolveBoxing());
    return super.visit(node);
  }

  @Override
  public boolean visit(FieldDeclaration node) {
    String declaringClassName = "";
    ASTNode parent = node.getParent();

    if (parent instanceof TypeDeclaration) {
      declaringClassName =
        ((TypeDeclaration) parent).resolveBinding().getQualifiedName();
    } else if (parent instanceof EnumDeclaration) {
      declaringClassName =
        ((EnumDeclaration) parent).resolveBinding().getQualifiedName();
    }

    String qualifiedType = node.getType().resolveBinding().getQualifiedName();

    for (Object varDeclFragObj : node.fragments()) {
      visitFieldDeclarationFragment(
        (VariableDeclarationFragment) varDeclFragObj,
        declaringClassName, qualifiedType);
    }
    return super.visit(node);
  }

  @Override
  public boolean visit(ImportDeclaration node) {
    // System.out.println(node);
    // mindkét mező egy fileid (vagy nem)

    /*
    JavaImport _import = new JavaImport();
    _import.setImporter(...);
    _import.setImported(...);

    persistRow(_import);
    */

    return super.visit(node);
  }

  @Override
  public boolean visit(InstanceofExpression node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(IntersectionType node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(LabeledStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(LambdaExpression node) {
    JavaMethod javaMethod = new JavaMethod();
    IMethodBinding methodBinding = node.resolveMethodBinding();
    ITypeBinding classBinding = methodBinding.getDeclaringClass();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr = methodBinding.toString();
    String qualifiedType = methodBinding.getReturnType().getQualifiedName();
    String entityHashStr = String.join(
      " ", declaringClassName, methodBindingStr);
    int modifiers = methodBinding.getModifiers();
    int entityHash = entityHashStr.hashCode();
    int classHash = declaringClassName.hashCode();
    int typeHash = qualifiedType.hashCode();

    setJavaTypedEntityFields(javaMethod, modifiers, typeHash, qualifiedType);

    // persist method's parameters
    for (Object varDeclObj : node.parameters()) {
      visitParameterDeclaration(
        (VariableDeclarationFragment) varDeclObj,
        entityHashStr, javaMethod::addJavaMetVarParam
      );
    }

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.METHOD, AstType.DEFINITION, entityHash);

    persistJavaMemberType(
      classHash, typeHash, MemberTypeKind.METHOD, modifiers, javaAstNode);

    setJavaEntityFields(javaMethod, javaAstNode.getId(), entityHash, "", "");

    persistRow(javaMethod);
    return super.visit(node);
  }

  @Override
  public boolean visit(MarkerAnnotation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(MemberRef node) {
    // System.out.println("====================================");
    // System.out.println(node.resolveBinding().getJavaElement());
    /*
    JavaMethod javaMethod = new JavaMethod();
    ITypeBinding classBinding =
      node.getDeclaringClass();
    IMethodBinding methodDeclBinding =
      node.resolveMethodBinding().getMethodDeclaration();
    SimpleName simpleName = node.getName();
    String qualifiedName = simpleName.getFullyQualifiedName();
    String qualifiedType = methodDeclBinding.getReturnType().getQualifiedName();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr = methodDeclBinding.toString();
    String entityHashStr = String.join(
      " ", declaringClassName, methodBindingStr);
    int entityHash = entityHashStr.hashCode();
    int typeHash = qualifiedType.hashCode();

    javaMethod.setTypeHash(typeHash);
    javaMethod.setQualifiedType(qualifiedType);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.METHOD, AstType.USAGE, entityHash);

    setJavaEntityFields(
      javaMethod, JavaAstNode.getId(), entityHash,
      simpleName.toString(), qualifiedName
    );

    persistRow(javaMethod);
    // System.out.println(node);
     */
    return super.visit(node);
  }

  @Override
  public boolean visit(MemberValuePair node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(MethodDeclaration node) {
    if (node.isConstructor()) {
      visitConstructorDeclaration(node);
    } else {
      visitMethodDeclaration(node);
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(MethodInvocation node) {
    JavaMethod javaMethod = new JavaMethod();
    IMethodBinding methodBinding =  node.resolveMethodBinding();
    ITypeBinding classBinding = methodBinding.getDeclaringClass();
    IMethodBinding methodDeclBinding = methodBinding.getMethodDeclaration();
    SimpleName simpleName = node.getName();
    String qualifiedName = simpleName.getFullyQualifiedName();
    String qualifiedType = methodDeclBinding.getReturnType().getQualifiedName();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr = methodDeclBinding.toString();
    String entityHashStr = String.join(
      " ", declaringClassName, methodBindingStr);
    int modifiers = methodBinding.getModifiers();
    int classHash = declaringClassName.hashCode();
    int entityHash = entityHashStr.hashCode();
    int typeHash = qualifiedType.hashCode();

    setJavaTypedEntityFields(javaMethod, modifiers, typeHash, qualifiedType);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.METHOD, AstType.USAGE, entityHash);

    // Set JavaMemberType fields
    persistJavaMemberType(
      classHash, typeHash, MemberTypeKind.METHOD, modifiers, javaAstNode);

    setJavaEntityFields(
      javaMethod, javaAstNode.getId(), entityHash,
      simpleName.toString(), qualifiedName
    );

    persistRow(javaMethod);

    return super.visit(node);
  }

  @Override
  public boolean visit(MethodRef node) {
    // System.out.println("''''''''''''''''''''''''''''''''''''");
    // System.out.println(node.resolveBinding().getJavaElement());
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(MethodRefParameter node) {
    // System.out.println("==============================");
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(Modifier node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ModuleDeclaration node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ModuleModifier node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(NameQualifiedType node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(NormalAnnotation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(OpensDirective node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(PackageDeclaration node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ParameterizedType node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ProvidesDirective node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(QualifiedType node) {
    // talán ez is kell
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(SimpleName node) {
    IBinding nodeBinding = node.resolveBinding();
    ASTNode parent = node.getParent();
    boolean isDeclaration =
      parent instanceof EnumConstantDeclaration ||
      parent instanceof SingleVariableDeclaration ||
      parent instanceof VariableDeclarationFragment;

    if (nodeBinding != null && nodeBinding.getKind() == IBinding.VARIABLE &&
        !isDeclaration)
    {
      IVariableBinding variableBinding =
        (IVariableBinding) node.resolveBinding();

      if (variableBinding.isEnumConstant()) {
        visitEnumConstantUsage(variableBinding, node);
      } else {
        visitVariableUsage(parent, variableBinding, node);
      }
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(SimpleType node) {
    visitTypeUsage(node, node.resolveBinding(), node.getName().toString());

    return super.visit(node);
  }

  @Override
  public boolean visit(SingleMemberAnnotation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(SuperConstructorInvocation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(SuperFieldAccess node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(SuperMethodInvocation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(SuperMethodReference node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ThisExpression node) {
    ITypeBinding typeBinding = node.resolveTypeBinding();

    visitTypeUsage(node, typeBinding, typeBinding.getName());

    return super.visit(node);
  }

  @Override
  public boolean visit(TypeDeclaration node) {
    JavaRecord javaRecord = new JavaRecord();
    SimpleName simpleName = node.getName();
    String qualifiedName = node.resolveBinding().getQualifiedName();
    int modifiers = node.getModifiers();
    int entityHash = qualifiedName.hashCode();

    setJavaRecordFields(javaRecord, modifiers);

    JavaAstNode javaAstNode =
      persistJavaAstNodeRow(
        node, SymbolType.TYPE, AstType.DEFINITION, entityHash);

    persistJavaMemberType(
      entityHash, entityHash, MemberTypeKind.TYPE, modifiers, javaAstNode);

    setJavaEntityFields(
      javaRecord, javaAstNode.getId(), entityHash,
      simpleName.toString(), qualifiedName
    );

    persistClassExtensions(node.getSuperclassType(), entityHash);
    persistInterfaceImplementations(node.superInterfaceTypes(), entityHash);

    persistRow(javaRecord);

    return super.visit(node);
  }

  @Override
  public boolean visit(TypeDeclarationStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(TypeLiteral node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(TypeMethodReference node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(TypeParameter node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(VariableDeclarationExpression node) {
    // Például forciklus ciklusváltozó
    // System.out.println("EXPRESSION");
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(VariableDeclarationFragment node) {
    IVariableBinding nodeBinding = node.resolveBinding();
    ASTNode parent = node.getParent();

    if (!nodeBinding.isField() && !(parent instanceof LambdaExpression)) {
      visitLocalVarDeclaration(node);
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(VariableDeclarationStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  private void visitTypeUsage(
    ASTNode node, ITypeBinding typeBinding, String name)
  {
    JavaRecord javaRecord = new JavaRecord();
    String qualifiedName = typeBinding.getQualifiedName();
    boolean isEnum = typeBinding.isEnum();
    int modifiers = typeBinding.getModifiers();
    int entityHash = qualifiedName.hashCode();

    setJavaRecordFields(javaRecord, modifiers);

    JavaAstNode javaAstNode =
      persistJavaAstNodeRow(
        node, isEnum ? SymbolType.ENUM : SymbolType.TYPE,
        AstType.USAGE, entityHash
      );

    persistJavaMemberType(
      entityHash, entityHash,
      isEnum ? MemberTypeKind.ENUM : MemberTypeKind.TYPE,
      modifiers, javaAstNode
    );

    setJavaEntityFields(
      javaRecord, javaAstNode.getId(), entityHash, name, qualifiedName);

    persistRow(javaRecord);
  }

  private void visitEnumConstantDeclaration(
    JavaEnum javaEnum, EnumConstantDeclaration node, int index)
  {
    JavaEnumConstant enumConstant = new JavaEnumConstant();
    ITypeBinding declaringClass =
      node.resolveConstructorBinding().getDeclaringClass();
    SimpleName simpleName = node.getName();
    String qualifiedName = simpleName.getFullyQualifiedName();
    String declaringClassName = declaringClass.getQualifiedName();
    String entityHashStr = String.join(
      " ", declaringClassName, qualifiedName);
    int modifiers = declaringClass.getModifiers();
    int entityHash = entityHashStr.hashCode();

    enumConstant.setValue(index);
    javaEnum.addJavaEnumConstant(enumConstant);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.ENUM_CONSTANT,
      AstType.DEFINITION, entityHash);

    // Set JavaMemberType fields
    persistJavaMemberType(
      entityHash, entityHash,
      MemberTypeKind.ENUM_CONSTANT, modifiers, javaAstNode
    );

    setJavaEntityFields(
      enumConstant, javaAstNode.getId(),
      entityHash, simpleName.toString(), qualifiedName
    );
  }

  private void visitFieldDeclarationFragment(
    VariableDeclarationFragment node,
    String declaringClassName, String qualifiedType)
  {
    JavaVariable javaVariable = new JavaVariable();
    IVariableBinding nodeBinding = node.resolveBinding();
    SimpleName simpleName = node.getName();
    String qualifiedName = simpleName.getFullyQualifiedName();
    String entityHashStr = String.join(
      " ", declaringClassName, qualifiedType, qualifiedName);
    int modifiers = nodeBinding.getModifiers();
    int entityHash = entityHashStr.hashCode();
    int classHash = declaringClassName.hashCode();
    int typeHash = qualifiedType.hashCode();

    setJavaTypedEntityFields(javaVariable, modifiers, typeHash, qualifiedType);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE,
      node.getInitializer() == null ? AstType.DECLARATION : AstType.DEFINITION,
      entityHash
    );

    persistJavaMemberType(
      classHash, typeHash, MemberTypeKind.FIELD, modifiers, javaAstNode);

    setJavaEntityFields(
      javaVariable, javaAstNode.getId(),
      entityHash, simpleName.toString(), qualifiedName
    );

    persistRow(javaVariable);
  }

  private void visitParameterDeclaration(
    VariableDeclaration node, String entityHashStrPrefix,
    Consumer<JavaVariable> connectParent)
  {
    JavaVariable javaVariable = new JavaVariable();
    IVariableBinding nodeBinding = node.resolveBinding();
    SimpleName simpleName = node.getName();
    String qualifiedType = nodeBinding.getType().getQualifiedName();
    String qualifiedName = simpleName.getFullyQualifiedName();
    String entityHashStr = String.join(
      " ", entityHashStrPrefix, qualifiedType, qualifiedName);
    int modifiers = nodeBinding.getModifiers();
    int entityHash = entityHashStr.hashCode();
    int typeHash = qualifiedType.hashCode();

    setJavaTypedEntityFields(javaVariable, modifiers, typeHash, qualifiedType);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE, AstType.DECLARATION, entityHash);

    setJavaEntityFields(
      javaVariable, javaAstNode.getId(),
      entityHash, simpleName.toString(), qualifiedName
    );

    connectParent.accept(javaVariable);
  }

  private void visitLocalVarDeclaration(
    VariableDeclaration node)
  {
    JavaVariable javaVariable = new JavaVariable();
    IVariableBinding nodeBinding = node.resolveBinding();
    SimpleName simpleName = node.getName();
    IMethodBinding methodDeclBinding = nodeBinding.getDeclaringMethod();
    ITypeBinding classBinding = methodDeclBinding.getDeclaringClass();
    String qualifiedName = simpleName.getFullyQualifiedName();
    String qualifiedType = nodeBinding.getType().getQualifiedName();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr = methodDeclBinding.toString();
    String methodEntityHashStr = String.join(
      " ", declaringClassName, methodBindingStr);
    String entityHashStr = String.join(
      " ",declaringClassName, methodBindingStr,
      qualifiedType, qualifiedName);
    int modifiers = nodeBinding.getModifiers();
    int methodEntityHash = methodEntityHashStr.hashCode();
    int entityHash = entityHashStr.hashCode();
    int typeHash = qualifiedType.hashCode();

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE,
      node.getInitializer() == null ? AstType.DECLARATION : AstType.DEFINITION,
      entityHash
    );

    JavaAstNode javaAstNodeDef =
      queryParentAstNode(javaAstNode, methodEntityHash);

    setJavaTypedEntityFields(javaVariable, modifiers, typeHash, qualifiedType);

    if (methodDeclBinding.isConstructor()) {
      CriteriaQuery<JavaConstructor> cr = cb.createQuery(JavaConstructor.class);
      Root<JavaConstructor> root = cr.from(JavaConstructor.class);

      JavaConstructor javaConstructor =
        queryJavaEntityByAstNodeId(cr, root, javaAstNodeDef.getId());

      javaConstructor.addJavaConVarLocal(javaVariable);
    } else {
      CriteriaQuery<JavaMethod> cr = cb.createQuery(JavaMethod.class);
      Root<JavaMethod> root = cr.from(JavaMethod.class);

      JavaMethod javaMethod =
        queryJavaEntityByAstNodeId(cr, root, javaAstNodeDef.getId());

      javaMethod.addJavaMetVarLocal(javaVariable);
    }

    setJavaEntityFields(
      javaVariable, javaAstNode.getId(), entityHash,
      simpleName.toString(), qualifiedName
    );

    persistRow(javaVariable);
  }

  private void visitConstructorDeclaration(MethodDeclaration node) {
    JavaConstructor javaConstructor = new JavaConstructor();
    IMethodBinding nodeBinding = node.resolveBinding();
    ITypeBinding classBinding = nodeBinding.getDeclaringClass();
    IMethodBinding methodDeclBinding = nodeBinding.getMethodDeclaration();
    SimpleName simpleName = node.getName();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr =  methodDeclBinding.toString();
    String entityHashStr = String.join(
      " ", declaringClassName, methodBindingStr);
    int modifiers = node.getModifiers();
    int entityHash = entityHashStr.hashCode();
    int classHash = declaringClassName.hashCode();

    // Persist constructor's parameters
    for (Object varDeclObj : node.parameters()) {
      visitParameterDeclaration(
        (SingleVariableDeclaration) varDeclObj,
        entityHashStr, javaConstructor::addJavaConVarParam
      );
    }

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.CONSTRUCTOR, AstType.DEFINITION, entityHash);

    persistJavaMemberType(
      classHash, classHash, MemberTypeKind.CONSTRUCTOR, modifiers, javaAstNode);

    setJavaEntityFields(
      javaConstructor, javaAstNode.getId(), entityHash,
      simpleName.toString(), declaringClassName
    );

    persistRow(javaConstructor);
  }

  private void visitMethodDeclaration(MethodDeclaration node) {
    JavaMethod javaMethod = new JavaMethod();
    IMethodBinding nodeBinding = node.resolveBinding();
    ITypeBinding classBinding = nodeBinding.getDeclaringClass();
    IMethodBinding methodDeclBinding = nodeBinding.getMethodDeclaration();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr = methodDeclBinding.toString();
    SimpleName simpleName = node.getName();
    String qualifiedName = simpleName.getFullyQualifiedName();
    String qualifiedType =
      node.getReturnType2().resolveBinding().getQualifiedName();
    AstType astType =
      classBinding.isInterface() ? AstType.DECLARATION : AstType.DEFINITION;
    String entityHashStr = String.join(
      " ", declaringClassName, methodBindingStr);
    int modifiers = node.getModifiers();
    int entityHash = entityHashStr.hashCode();
    int classHash = declaringClassName.hashCode();
    int typeHash = qualifiedType.hashCode();

    setJavaTypedEntityFields(javaMethod, modifiers, typeHash, qualifiedType);

    // persist method's parameters
    for (Object varDeclObj : node.parameters()) {
      visitParameterDeclaration(
        (SingleVariableDeclaration) varDeclObj,
        entityHashStr, javaMethod::addJavaMetVarParam
      );
    }

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.METHOD, astType, entityHash);

    persistJavaMemberType(
      classHash, typeHash, MemberTypeKind.METHOD, modifiers, javaAstNode);

    setJavaEntityFields(
      javaMethod, javaAstNode.getId(), entityHash,
      simpleName.toString(), qualifiedName
    );

    persistRow(javaMethod);
  }

  private void visitEnumConstantUsage(
    IVariableBinding variableBinding, SimpleName node)
  {
    JavaEnumConstant javaEnumConstant = new JavaEnumConstant();
    String qualifiedType = variableBinding.getType().getQualifiedName();
    String name = node.toString();
    String qualifiedName = node.getFullyQualifiedName();
    String entityHashStr = String.join(
      " ", qualifiedType, name);
    int modifiers = variableBinding.getModifiers();
    int classHash = qualifiedType.hashCode();
    int entityHash = entityHashStr.hashCode();

    javaEnumConstant.setValue(variableBinding.getVariableId());

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.ENUM_CONSTANT, AstType.USAGE, entityHash
    );

    persistJavaMemberType(
      classHash, classHash,
      MemberTypeKind.ENUM_CONSTANT, modifiers, javaAstNode
    );

    setJavaEntityFields(
      javaEnumConstant, javaAstNode.getId(), entityHash, name, qualifiedName);

    persistRow(javaEnumConstant);
  }

  private void visitVariableUsage(
    ASTNode parent, IVariableBinding variableBinding, SimpleName node)
  {
    JavaVariable javaVariable = new JavaVariable();
    String qualifiedType = variableBinding.getType().getQualifiedName();
    String name = node.toString();
    String qualifiedName = node.getFullyQualifiedName();
    String entityHashStr = "";
    String declaringClassName;
    AstType astType;

    if (variableBinding.isField()) {
      ITypeBinding classBinding = variableBinding.getDeclaringClass();
      declaringClassName = classBinding.getQualifiedName();
      entityHashStr = String.join(
        " ", declaringClassName, qualifiedType, name);
    } else {
      IMethodBinding declaringMethodBinding =
        variableBinding.getVariableDeclaration().getDeclaringMethod();
      ITypeBinding classBinding = declaringMethodBinding.getDeclaringClass();
      String methodBindingStr = declaringMethodBinding.toString();
      declaringClassName = classBinding.getQualifiedName();
      entityHashStr = String.join(
        " ", declaringClassName,
        methodBindingStr, qualifiedType, name);
    }

    int modifiers = variableBinding.getModifiers();
    int classHash = declaringClassName.hashCode();
    int entityHash = entityHashStr.hashCode();
    int typeHash = qualifiedType.hashCode();

    setJavaTypedEntityFields(javaVariable, modifiers, typeHash, qualifiedType);

    if (parent instanceof Assignment) {
      Assignment assignment = (Assignment) parent;
      Expression leftHandSide = assignment.getLeftHandSide();

      astType =
        node.getStartPosition() == leftHandSide.getStartPosition() ?
          AstType.WRITE :
          AstType.READ;
    } else if (parent instanceof MethodInvocation) {
      MethodInvocation methodInvocation = (MethodInvocation) parent;
      astType =
        node.getStartPosition() == methodInvocation.getStartPosition() ?
          AstType.WRITE :
          AstType.READ;
    } else if (
      parent instanceof PrefixExpression ||
      parent instanceof PostfixExpression)
    {
      astType = AstType.WRITE;
    } else {
      astType = AstType.READ;
    }

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE, astType, entityHash);

    if (variableBinding.isField()) {
      persistJavaMemberType(
        classHash, typeHash, MemberTypeKind.FIELD, modifiers, javaAstNode);
    }

    setJavaEntityFields(
      javaVariable, javaAstNode.getId(), entityHash, name, qualifiedName);

    persistRow(javaVariable);
  }

  private void visitJavaDoc(Javadoc node, int entityHash) {
    JavaDocComment javaDocComment = new JavaDocComment();
    String commentString = node.toString();

    setJavaDocCommentFields(javaDocComment, commentString, entityHash);

    persistRow(javaDocComment);
  }

  private JavaAstNode queryParentAstNode(
    JavaAstNode child, int methodEntityHash)
  {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    long childLocationFile = child.getLocation_file();
    long childStartLine = child.getLocation_range_start_line();
    long childStartColumn = child.getLocation_range_start_column();
    Path<Long> locationFile =
      root.get("location_file");
    Path<Long> entityHash =
      root.get("entityHash");
    Path<Long> astType =
      root.get("astType");
    Path<Integer> startLine =
      root.get("location_range_start_line");
    Path<Integer> endLine =
      root.get("location_range_end_line");
    Path<Integer> startColumn =
      root.get("location_range_start_column");
    Path<Integer> endColumn =
      root.get("location_range_end_column");

    Predicate sameFile = cb.equal(locationFile, childLocationFile);
    Predicate sameEntityHash = cb.equal(entityHash, methodEntityHash);
    Predicate definition = cb.equal(astType, AstType.DEFINITION);
    Predicate startPosLessEqualPos =
      cb.or(
        cb.and(
          cb.equal(startLine, childStartLine),
          cb.le(startColumn, childStartColumn)
        ),
        cb.lt(startLine, childStartLine)
      );
    Predicate posLessThanEndPos =
      cb.or(
        cb.and(
          cb.equal(endLine, childStartLine),
          cb.gt(endColumn, childStartColumn)
        ),
        cb.gt(endLine, childStartLine)
      );

    cr
      .select(root)
      .where(
        cb.and(
          sameFile, sameEntityHash, definition,
          startPosLessEqualPos, posLessThanEndPos
        )
      );

    return em.createQuery(cr).getSingleResult();
  }

  private <E extends JavaEntity> E queryJavaEntityByAstNodeId(
    CriteriaQuery<E> cr,
    Root<E> root,
    long astNodeId)
  {
    cr
      .select(root)
      .where(cb.equal(root.get("astNodeId"), astNodeId));

    return em.createQuery(cr).getSingleResult();
  }

  private Visibility getVisibility(int modifiers) {
    if (Flags.isPublic(modifiers)) {
      return Visibility.PUBLIC;
    } else if (Flags.isProtected(modifiers)) {
      return Visibility.PROTECTED;
    } else if (Flags.isPrivate(modifiers)) {
      return Visibility.PRIVATE;
    }

    return Visibility.PACKAGE_PRIVATE;
  }

  private void setJavaDocCommentFields(
    JavaDocComment javaDocComment, String commentString, long entityHash)
  {
    javaDocComment.setContent(commentString);
    javaDocComment.setContentHash(commentString.hashCode());
    javaDocComment.setEntityHash(entityHash);
  }

  private void setJavaRecordFields(JavaRecord javaRecord, int modifiers) {
    javaRecord.setAbstract(Flags.isAbstract(modifiers));
    javaRecord.setFinal(Flags.isFinal(modifiers));
  }

  private <T extends JavaTypedEntity> void setJavaTypedEntityFields(
    T javaTypedEntity, int modifiers, long typeHash, String qualifiedType)
  {
    javaTypedEntity.setFinal(Flags.isFinal(modifiers));
    javaTypedEntity.setStatic(Flags.isStatic(modifiers));
    javaTypedEntity.setTypeHash(typeHash);
    javaTypedEntity.setQualifiedType(qualifiedType);
  }

  private <T extends JavaEntity> void setJavaEntityFields(
    T javaEntity, long astNodeId,
    long entityHash,String name, String qualifiedName)
  {
    javaEntity.setAstNodeId(astNodeId);
    javaEntity.setEntityHash(entityHash);
    javaEntity.setName(name);
    javaEntity.setQualifiedName(qualifiedName);
  }

  private void persistClassExtensions(
    Type superclassType, int entityHash)
  {
    if (superclassType != null) {
      String qualifiedSuperClassName =
        superclassType.resolveBinding().getQualifiedName();
      int superClassHash = qualifiedSuperClassName.hashCode();

      persistJavaInheritance(superClassHash, entityHash);
    }
  }

  private void persistInterfaceImplementations(
    List<?> superInterfaceTypes, int entityHash)
  {
    superInterfaceTypes.forEach(i -> {
      Type aInterface = (Type) i;
      String qualifiedSuperInterfaceName =
        aInterface.resolveBinding().getQualifiedName();
      int superInterfaceHash = qualifiedSuperInterfaceName.hashCode();

      persistJavaInheritance(superInterfaceHash, entityHash);
    });
  }

  private JavaInheritance persistJavaInheritance(
    int baseEntityHash, int derivedEntityHash)
  {
    JavaInheritance javaInheritance = new JavaInheritance();

    javaInheritance.setBase(baseEntityHash);
    javaInheritance.setDerived(derivedEntityHash);

    persistRow(javaInheritance);

    return javaInheritance;
  }

  private JavaMemberType persistJavaMemberType(
    long typeHash, long memberTypeHash,
    MemberTypeKind memberTypeKind, int modifiers, JavaAstNode javaAstNode)
  {
    JavaMemberType javaMemberType = new JavaMemberType();
    javaMemberType.setTypeHash(typeHash);
    javaMemberType.setMemberTypeHash(memberTypeHash);
    javaMemberType.setKind(memberTypeKind);
    javaMemberType.setVisibility(getVisibility(modifiers));
    javaMemberType.setMemberAstNode(javaAstNode);

    persistRow(javaMemberType);

    return javaMemberType;
  }

  private <T extends ASTNode> JavaAstNode persistJavaAstNodeRow(
    T node, SymbolType symbolType, AstType astType, int entityHash)
  {
    JavaAstNode javaAstNode = new JavaAstNode();
    PositionInfo positionInfo;

    javaAstNode.setLocation_file(fileId);
    // visibleinsourcecode: akkor lesz false, ha az adott függvényt, vagy akármit
    // nem közvetlenül hívjuk a forráskódból, hanem hívunk valamit egy libraryből, ami meghívja aztán ezt
    try {
      Method getJavadocMethod =
              node.getClass().getMethod("getJavadoc", (Class<?>[]) null);
      Javadoc javadoc =
              (Javadoc) getJavadocMethod.invoke(node, (Object[]) null);

      visitJavaDoc(javadoc, entityHash);

      positionInfo = new PositionInfo(this.cu, node, javadoc);
      int javadocLen = javadoc.toString().length();
      javaAstNode.setAstValue(
              node.toString().substring(javadocLen)
      );

      javaAstNode.setLocation_range_start_line(positionInfo.getStartLine());
      javaAstNode.setLocation_range_start_column(positionInfo.getStartColumn());
      javaAstNode.setLocation_range_end_line(positionInfo.getEndLine());
      javaAstNode.setLocation_range_end_column(positionInfo.getEndColumn());

    } catch (NoSuchMethodException | NullPointerException e) {
      positionInfo = new PositionInfo(this.cu, node);

      javaAstNode.setAstValue(node.toString());
      javaAstNode.setLocation_range_start_line(positionInfo.getStartLine());
      javaAstNode.setLocation_range_start_column(positionInfo.getStartColumn());
      javaAstNode.setLocation_range_end_line(positionInfo.getEndLine());
      javaAstNode.setLocation_range_end_column(positionInfo.getEndColumn());

    } catch (IllegalAccessException | InvocationTargetException e) {
      e.printStackTrace();
    }
    javaAstNode.setEntityHash(entityHash);
    javaAstNode.setSymbolType(symbolType);
    javaAstNode.setAstType(astType);
    javaAstNode.setVisibleInSourceCode(true);

    persistRow(javaAstNode);

    return javaAstNode;
  }

  private void persistRow(Object jpaObject) {
    em.getTransaction().begin();
    em.persist(jpaObject);
    em.getTransaction().commit();
  }
}
