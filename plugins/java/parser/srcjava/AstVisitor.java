package parser.srcjava;

import model.*;
import model.enums.*;
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
    // visitConstructorUsage(node, node.resolveTypeBinding().getDeclaringMethod());

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
  public boolean visit(CatchClause node) {
    SingleVariableDeclaration variableDeclaration = node.getException();

    visitLocalVarDeclaration(variableDeclaration);

    return super.visit(node);
  }

  @Override
  public boolean visit(ClassInstanceCreation node) {
    visitConstructorUsage(node, node.resolveConstructorBinding());

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
    List<?> superInterfaceTypes = node.superInterfaceTypes();
    List<?> enumConstants = node.enumConstants();
    SimpleName simpleName = node.getName();
    String declaringClassName = node.resolveBinding().getQualifiedName();
    int modifiers = node.getModifiers();
    int entityHash = declaringClassName.hashCode();

    // Persist Enum constants
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

    persistInterfaceImplementations(superInterfaceTypes, entityHash);

    persistRow(javaEnum);

    return super.visit(node);
  }

  @Override
  public boolean visit(ExpressionMethodReference node) {
    visitMethodUsage(node, node.resolveMethodBinding(), node.getName());

    return super.visit(node);
  }

  @Override
  public boolean visit(FieldAccess node) {
    visitVariableUsage(node, node.resolveFieldBinding());

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
    // mindkét mező egy fileid (vagy nem)
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(LambdaExpression node) {
    JavaMethod javaMethod = new JavaMethod();
    IMethodBinding methodBinding = node.resolveMethodBinding();
    ITypeBinding classBinding = methodBinding.getDeclaringClass();
    String declaringClassName = classBinding.getQualifiedName();
    String qualifiedName = methodBinding.getName();
    String qualifiedType = methodBinding.getReturnType().getQualifiedName();
    String parametersStr =
      Arrays.toString(
        getParameterTypeNames(methodBinding.getParameterTypes()));
    String entityHashStr = String.join(
      " ", declaringClassName, qualifiedType,
      qualifiedName, parametersStr
    );
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
    visitMethodUsage(node, node.resolveMethodBinding(), node.getName());

    return super.visit(node);
  }

  @Override
  public boolean visit(ModuleDeclaration node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(NormalAnnotation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(PackageDeclaration node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(QualifiedName node) {
    IBinding nodeBinding = node.resolveBinding();
    ASTNode parent = node.getParent();
    boolean isDeclaration =
      parent instanceof EnumConstantDeclaration ||
        parent instanceof SingleVariableDeclaration ||
        parent instanceof VariableDeclarationFragment;
    boolean isFieldAccess =
      parent instanceof FieldAccess ||
        parent instanceof SuperFieldAccess;

    if (nodeBinding != null && nodeBinding.getKind() == IBinding.VARIABLE &&
      !isDeclaration && !isFieldAccess)
    {
      IVariableBinding variableBinding =
        (IVariableBinding) node.resolveBinding();

      if (variableBinding.isEnumConstant()) {
        visitEnumConstantUsage(node, variableBinding);
      } else {
        visitVariableUsage(node, variableBinding);
      }
    }

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
    boolean isFieldAccess =
      parent instanceof FieldAccess ||
      parent instanceof SuperFieldAccess;
    boolean isPartOfQualifiedName =
      parent instanceof QualifiedName;

    if (nodeBinding != null && nodeBinding.getKind() == IBinding.VARIABLE &&
      !isDeclaration && !isFieldAccess && !isPartOfQualifiedName)
    {
      IVariableBinding variableBinding =
        (IVariableBinding) node.resolveBinding();

      if (variableBinding.isEnumConstant()) {
        visitEnumConstantUsage(node, variableBinding);
      } else {
        visitVariableUsage(node, variableBinding);
      }
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(SimpleType node) {
    JavaRecord javaRecord = new JavaRecord();
    ITypeBinding typeBinding = node.resolveBinding();
    Name name = node.getName();
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
      javaRecord, javaAstNode.getId(),
      entityHash, name.toString(), qualifiedName
    );

    persistRow(javaRecord);

    return super.visit(node);
  }

  @Override
  public boolean visit(SingleMemberAnnotation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(SuperConstructorInvocation node) {
    visitConstructorUsage(node, node.resolveConstructorBinding());

    return super.visit(node);
  }

  @Override
  public boolean visit(SuperFieldAccess node) {
    visitVariableUsage(node, node.resolveFieldBinding());

    return super.visit(node);
  }

  @Override
  public boolean visit(SuperMethodInvocation node) {
    visitMethodUsage(node, node.resolveMethodBinding(), node.getName());

    return super.visit(node);
  }

  @Override
  public boolean visit(SuperMethodReference node) {
    visitMethodUsage(node, node.resolveMethodBinding(), node.getName());

    return super.visit(node);
  }

  @Override
  public boolean visit(ThisExpression node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(TypeDeclaration node) {
    JavaRecord javaRecord = new JavaRecord();
    Type superclassType = node.getSuperclassType();
    List<?> superInterfaceTypes = node.superInterfaceTypes();
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

    persistClassExtensions(superclassType, entityHash);
    persistInterfaceImplementations(superInterfaceTypes, entityHash);

    persistRow(javaRecord);

    return super.visit(node);
  }

  @Override
  public boolean visit(TypeMethodReference node) {
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

  private void visitConstructorUsage(
    ASTNode node, IMethodBinding constructorBinding)
  {
    JavaConstructor javaConstructor = new JavaConstructor();
    ITypeBinding classBinding = constructorBinding.getDeclaringClass();
    String name = constructorBinding.getName();
    String qualifiedName = classBinding.getQualifiedName();
    String methodQualifiedType =
      constructorBinding.getReturnType().getQualifiedName();
    String parametersStr =
      Arrays.toString(
        getParameterTypeNames(constructorBinding.getParameterTypes()));
    String entityHashStr = String.join(
      " ", qualifiedName, methodQualifiedType, name, parametersStr);
    int modifiers = constructorBinding.getModifiers();
    int classHash = qualifiedName.hashCode();
    int entityHash = entityHashStr.hashCode();

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.CONSTRUCTOR, AstType.USAGE, entityHash);

    persistJavaMemberType(
      classHash, classHash, MemberTypeKind.CONSTRUCTOR, modifiers, javaAstNode);

    setJavaEntityFields(
      javaConstructor, javaAstNode.getId(),
      entityHash, name, qualifiedName
    );

    persistRow(javaConstructor);
  }

  private void visitMethodUsage(
    Expression node, IMethodBinding methodBinding, SimpleName simpleName)
  {
    JavaMethod javaMethod = new JavaMethod();
    ITypeBinding classBinding = methodBinding.getDeclaringClass();
    String qualifiedName = simpleName.getFullyQualifiedName();
    String qualifiedType = methodBinding.getReturnType().getQualifiedName();
    String declaringClassName = classBinding.getQualifiedName();
    String parametersStr =
      Arrays.toString(
        getParameterTypeNames(methodBinding.getParameterTypes()));
    String entityHashStr = String.join(
      " ", declaringClassName, qualifiedType,
      qualifiedName, parametersStr
    );
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
    String methodName = methodDeclBinding.getName();
    String methodType = methodDeclBinding.getReturnType().getQualifiedName();
    String parametersStr =
      Arrays.toString(
        getParameterTypeNames(methodDeclBinding.getParameterTypes()));
    String qualifiedName = simpleName.getFullyQualifiedName();
    String qualifiedType = nodeBinding.getType().getQualifiedName();
    String declaringClassName = classBinding.getQualifiedName();
    String methodEntityHashStr = String.join(
      " ", declaringClassName, methodType, methodName, parametersStr
    );
    String entityHashStr = String.join(
      " ", methodEntityHashStr, qualifiedType, qualifiedName
    );
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
    IMethodBinding methodBinding = node.resolveBinding();
    ITypeBinding classBinding = methodBinding.getDeclaringClass();
    String name = node.getName().toString();
    String methodQualifiedType =
      methodBinding.getReturnType().getQualifiedName();
    String declaringClassName = classBinding.getQualifiedName();
    String parametersStr =
      Arrays.toString(
        getParameterTypeNames(methodBinding.getParameterTypes()));
    String entityHashStr = String.join(
      " ", declaringClassName, methodQualifiedType, name, parametersStr
    );
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
      name, declaringClassName
    );

    persistRow(javaConstructor);
  }

  private void visitMethodDeclaration(MethodDeclaration node) {
    JavaMethod javaMethod = new JavaMethod();
    IMethodBinding methodBinding = node.resolveBinding();
    ITypeBinding classBinding = methodBinding.getDeclaringClass();
    String declaringClassName = classBinding.getQualifiedName();
    String parametersStr =
      Arrays.toString(
        getParameterTypeNames(methodBinding.getParameterTypes()));
    SimpleName simpleName = node.getName();
    String name = simpleName.toString();
    String qualifiedName = simpleName.getFullyQualifiedName();
    String qualifiedType =
      node.getReturnType2().resolveBinding().getQualifiedName();
    AstType astType =
      node.getBody() == null ? AstType.DECLARATION : AstType.DEFINITION;
    String entityHashStr = String.join(
      " ", declaringClassName, qualifiedType,
      qualifiedName, parametersStr
    );
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

    JavaMemberType javaMemberType = persistJavaMemberType(
      classHash, typeHash, MemberTypeKind.METHOD, modifiers, javaAstNode);

    persistMethodRelation(
      classBinding.getSuperclass(), classBinding.getInterfaces(),
      methodBinding, name, javaMemberType.getVisibility(), entityHash
    );

    setJavaEntityFields(
      javaMethod, javaAstNode.getId(), entityHash, name, qualifiedName);

    persistRow(javaMethod);
  }

  private void visitEnumConstantUsage(
    Name node, IVariableBinding variableBinding)
  {
    JavaEnumConstant javaEnumConstant = new JavaEnumConstant();
    String qualifiedType = variableBinding.getType().getQualifiedName();
    String name = variableBinding.getName();
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
      javaEnumConstant, javaAstNode.getId(), entityHash, name, name);

    persistRow(javaEnumConstant);
  }

  private void visitVariableUsage(
    ASTNode node, IVariableBinding variableBinding)
  {
    JavaVariable javaVariable = new JavaVariable();
    ASTNode parent = node.getParent();
    String qualifiedType = variableBinding.getType().getQualifiedName();
    String name = variableBinding.getName();
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
      String methodName = declaringMethodBinding.getName();
      String methodQualifiedType =
        declaringMethodBinding.getReturnType().getQualifiedName();
      String parametersStr =
        Arrays.toString(
          getParameterTypeNames(declaringMethodBinding.getParameterTypes()));
      declaringClassName = classBinding.getQualifiedName();
      entityHashStr = String.join(
        " ", declaringClassName, methodQualifiedType,
        methodName, parametersStr, qualifiedType, name
      );
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
      javaVariable, javaAstNode.getId(), entityHash, name, name);

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

  private String[] getParameterTypeNames(ITypeBinding[] parameters) {
    String[] parameterTypeNames = new String[parameters.length];

    for (int i = 0; i < parameters.length; i++) {
      parameterTypeNames[i] = parameters[i].getQualifiedName();
    }

    return parameterTypeNames;
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

  private boolean isOverriddenMethod(
    IMethodBinding superMethodBinding, IMethodBinding methodBinding,
    String methodName, Visibility methodVisibility)
  {
    Visibility mVisibility = getVisibility(superMethodBinding.getModifiers());
    String mName = superMethodBinding.getName();

    return mName.equals(methodName) &&
      mVisibility.getValue() <=
        methodVisibility.getValue() &&
      Arrays.equals(
        superMethodBinding.getParameterTypes(), methodBinding.getParameterTypes()
      );
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

  private JavaRelation persistMethodRelation(
    ITypeBinding superclassBinding, ITypeBinding[] superInterfaceBindings,
    IMethodBinding methodBinding, String methodName,
    Visibility methodVisibility, int methodEntityHash)
  {
    JavaRelation javaRelation;

    javaRelation = getMethodExtension(
      superclassBinding, methodBinding,
      methodName, methodVisibility, methodEntityHash
    );

    if (javaRelation == null) {
      javaRelation = getMethodImplementation(
        superInterfaceBindings, methodBinding,
        methodName, methodVisibility, methodEntityHash
      );
    }

    if (javaRelation != null) {
      persistRow(javaRelation);
    }

    return javaRelation;
  }

  private JavaRelation getMethodExtension(
    ITypeBinding superclassBinding, IMethodBinding methodBinding,
    String methodName, Visibility methodVisibility, int methodEntityHash)
  {
    if (superclassBinding == null) {
      return null;
    }

    IMethodBinding[] declaredMethods = superclassBinding.getDeclaredMethods();
    Optional<IMethodBinding> superMethodBinding =
      Arrays.stream(declaredMethods)
        .filter(m ->
          isOverriddenMethod(m, methodBinding, methodName, methodVisibility))
        .findFirst();

    if (superMethodBinding.isPresent()) {
      JavaRelation javaRelation = new JavaRelation();
      String superclassQualifiedName = superclassBinding.getQualifiedName();
      String methodQualifiedType =
        methodBinding.getReturnType().getQualifiedName();
      String parametersStr =
        Arrays.toString(
          getParameterTypeNames(methodBinding.getParameterTypes()));
      String superMethodEntityHashStr = String.join(
        " ", superclassQualifiedName,
        methodQualifiedType, methodName, parametersStr
      );
      int superMethodEntityHash = superMethodEntityHashStr.hashCode();

      setJavaRelationFields(
        javaRelation, superMethodEntityHash,
        methodEntityHash, RelationKind.OVERRIDE
      );

      return javaRelation;
    }

    return getMethodExtension(
      superclassBinding.getSuperclass(), methodBinding,
      methodName, methodVisibility, methodEntityHash
    );
  }

  private JavaRelation getMethodImplementation(
    ITypeBinding[] superInterfaceBindings, IMethodBinding methodBinding,
    String methodName, Visibility methodVisibility, int methodEntityHash)
  {
    return Arrays.stream(superInterfaceBindings)
      .map(
        i ->
          getMethodExtension(
            i, methodBinding, methodName, methodVisibility, methodEntityHash
          )
      )
      .filter(Objects::nonNull).findFirst().orElse(null);
  }

  private void setJavaRelationFields(
    JavaRelation javaRelation, int leftEntityHash,
    int rightEntityHash, RelationKind kind)
  {
    javaRelation.setLhs(leftEntityHash);
    javaRelation.setRhs(rightEntityHash);
    javaRelation.setKind(kind);
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
