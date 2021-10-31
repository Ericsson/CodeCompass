package parser.srcjava;

import model.*;
import model.enums.AstType;
import model.enums.MemberTypeKind;
import model.enums.SymbolType;
import model.enums.Visibility;
import org.eclipse.jdt.core.Flags;
import org.eclipse.jdt.core.dom.*;

import javax.persistence.EntityManager;
import javax.persistence.criteria.CriteriaBuilder;
import javax.persistence.criteria.CriteriaQuery;
import javax.persistence.criteria.Root;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;

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
  public boolean visit(ClassInstanceCreation node) {
    JavaConstructor javaConstructor = new JavaConstructor();
    IMethodBinding methodDeclBinding =
      node.resolveConstructorBinding().getMethodDeclaration();
    String qualifiedName = node.getType().resolveBinding().getQualifiedName();
    String entityHashStr = String.join(
      " ", qualifiedName, methodDeclBinding.toString());
    int entityHash = entityHashStr.hashCode();

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.CONSTRUCTOR, AstType.USAGE, entityHash);

    // Set JavaEntity fields
    javaConstructor.setAstNodeId(javaAstNode.getId());
    javaConstructor.setEntityHash(entityHash);
    javaConstructor.setName(node.getType().toString());
    javaConstructor.setQualifiedName(qualifiedName);

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
  public boolean visit(EnumDeclaration node) {
    JavaEnum javaEnum = new JavaEnum();
    SimpleName simpleName = node.getName();
    String declaringClassName = node.resolveBinding().getQualifiedName();
    int entityHash = declaringClassName.hashCode();

    // Persist Enum constants
    List<?> enumConstants = node.enumConstants();
    for (int i = 0; i < enumConstants.size(); i++) {
      visitEnumConstantDeclaration(
        javaEnum, (EnumConstantDeclaration) enumConstants.get(i), i);
    }

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.ENUM, AstType.DEFINITION, entityHash);

    // Set JavaEntity fields
    javaEnum.setAstNodeId(javaAstNode.getId());
    javaEnum.setEntityHash(entityHash);
    javaEnum.setName(simpleName.toString());
    javaEnum.setQualifiedName(declaringClassName);

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
    // System.out.println("AAAAAAAAAAAAAAAAAAAAAAA");
    // System.out.println(node.resolveMethodBinding().getMethodDeclaration());
    // System.out.println(node);
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

    // Set JavaEntity fields
    javaMethod.setAstNodeId(javaAstNode.getId());
    javaMethod.setEntityHash(entityHash);
    javaMethod.setName(simpleName.toString());
    javaMethod.setQualifiedName(qualifiedName);

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
    int entityHash = entityHashStr.hashCode();
    int typeHash = qualifiedType.hashCode();

    javaMethod.setFinal(Flags.isFinal(modifiers));
    javaMethod.setStatic(Flags.isStatic(modifiers));
    javaMethod.setTypeHash(typeHash);
    javaMethod.setQualifiedType(qualifiedType);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.METHOD, AstType.USAGE, entityHash);

    // Set JavaEntity fields
    javaMethod.setAstNodeId(javaAstNode.getId());
    javaMethod.setEntityHash(entityHash);
    javaMethod.setName(simpleName.toString());
    javaMethod.setQualifiedName(qualifiedName);

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

    javaRecord.setAbstract(Flags.isAbstract(modifiers));
    javaRecord.setFinal(Flags.isFinal(modifiers));

    JavaAstNode javaAstNode =
      persistJavaAstNodeRow(
        node, SymbolType.TYPE, AstType.DEFINITION, entityHash);

    // Set JavaEntity fields
    javaRecord.setAstNodeId(javaAstNode.getId());
    javaRecord.setEntityHash(entityHash);
    javaRecord.setName(simpleName.toString());
    javaRecord.setQualifiedName(qualifiedName);

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
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(VariableDeclarationStatement node) {
    // Method local variables
    String qualifiedType = node.getType().resolveBinding().getQualifiedName();

    for (Object varDeclFragObj : node.fragments()) {
      visitVariableDeclarationFragment(
        (VariableDeclarationFragment) varDeclFragObj, qualifiedType);
    }
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

    javaRecord.setAbstract(Flags.isAbstract(modifiers));
    javaRecord.setFinal(Flags.isFinal(modifiers));

    JavaAstNode javaAstNode =
      persistJavaAstNodeRow(
        node, isEnum ? SymbolType.ENUM : SymbolType.TYPE,
        AstType.USAGE, entityHash
      );

    // Set JavaEntity fields
    javaRecord.setAstNodeId(javaAstNode.getId());
    javaRecord.setEntityHash(entityHash);
    javaRecord.setName(name);
    javaRecord.setQualifiedName(qualifiedName);

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
    int ecnEntityHash = entityHashStr.hashCode();

    enumConstant.setValue(index);
    javaEnum.addJavaEnumConstant(enumConstant);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.ENUM_CONSTANT,
      AstType.DEFINITION, ecnEntityHash);

    // Set JavaEntity fields
    enumConstant.setAstNodeId(javaAstNode.getId());
    enumConstant.setEntityHash(ecnEntityHash);
    enumConstant.setName(simpleName.toString());
    enumConstant.setQualifiedName(qualifiedName);
  }

  private void visitFieldDeclarationFragment(
    VariableDeclarationFragment node,
    String declaringClassName, String qualifiedType)
  {
    JavaVariable javaVariable = new JavaVariable();
    JavaMemberType javaMemberType = new JavaMemberType();
    IVariableBinding nodeBinding = node.resolveBinding();
    SimpleName simpleName = node.getName();
    String qualifiedName = simpleName.getFullyQualifiedName();
    String entityHashStr = String.join(
      " ", declaringClassName, qualifiedType, qualifiedName);
    int modifiers = nodeBinding.getModifiers();
    int entityHash = entityHashStr.hashCode();
    int classHash = declaringClassName.hashCode();
    int typeHash = qualifiedType.hashCode();

    javaVariable.setFinal(Flags.isFinal(modifiers));
    javaVariable.setStatic(Flags.isStatic(modifiers));
    javaVariable.setTypeHash(typeHash);
    javaVariable.setQualifiedType(qualifiedType);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE,
      node.getInitializer() == null ? AstType.DECLARATION : AstType.DEFINITION,
      entityHash
    );

    // Set JavaMemberType fields
    javaMemberType.setTypeHash(classHash);
    javaMemberType.setMemberTypeHash(typeHash);
    javaMemberType.setKind(MemberTypeKind.FIELD);
    javaMemberType.setVisibility(getVisibility(modifiers));
    javaMemberType.setMemberAstNode(javaAstNode);

    // Set JavaEntity fields
    javaVariable.setAstNodeId(javaAstNode.getId());
    javaVariable.setEntityHash(entityHash);
    javaVariable.setName(simpleName.toString());
    javaVariable.setQualifiedName(qualifiedName);

    persistRow(javaMemberType);
    persistRow(javaVariable);
  }

  private void visitVariableDeclarationFragment(
    VariableDeclarationFragment node, String qualifiedType)
  {
    JavaVariable javaVariable = new JavaVariable();
    IVariableBinding nodeBinding = node.resolveBinding();
    SimpleName simpleName = node.getName();
    String qualifiedName = simpleName.getFullyQualifiedName();
    IMethodBinding declaringMethodBinding =
      node.resolveBinding().getDeclaringMethod();
    ITypeBinding classBinding = declaringMethodBinding.getDeclaringClass();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr = declaringMethodBinding.toString();
    String methodEntityHashStr = String.join(
      " ", declaringClassName, methodBindingStr);
    String entityHashStr = String.join(
      " ",declaringClassName, methodBindingStr,
      qualifiedType, qualifiedName);
    int modifiers = nodeBinding.getModifiers();
    int methodEntityHash = methodEntityHashStr.hashCode();
    int entityHash = entityHashStr.hashCode();
    int typeHash = qualifiedType.hashCode();
    List<JavaAstNode> javaAstNodeDefs =
      queryJavaAstNodeDefinitionsByEntityHash(methodEntityHash);
    JavaAstNode javaAstNodeDef = javaAstNodeDefs.get(0);

    javaVariable.setFinal(Flags.isFinal(modifiers));
    javaVariable.setStatic(Flags.isStatic(modifiers));
    javaVariable.setTypeHash(typeHash);
    javaVariable.setQualifiedType(qualifiedType);

    if (declaringMethodBinding.isConstructor()) {
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

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE,
      node.getInitializer() == null ? AstType.DECLARATION : AstType.DEFINITION,
      entityHash
    );

    // Set JavaEntity fields
    javaVariable.setAstNodeId(javaAstNode.getId());
    javaVariable.setEntityHash(entityHash);
    javaVariable.setName(simpleName.toString());
    javaVariable.setQualifiedName(qualifiedName);

    persistRow(javaVariable);
  }

  private void visitConstructorDeclaration(MethodDeclaration node) {
    JavaConstructor javaConstructor = new JavaConstructor();
    JavaMemberType javaMemberType = new JavaMemberType();
    IMethodBinding nodeBinding = node.resolveBinding();
    ITypeBinding classBinding = nodeBinding.getDeclaringClass();
    IMethodBinding methodDeclBinding = nodeBinding.getMethodDeclaration();
    SimpleName simpleName = node.getName();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr =  methodDeclBinding.toString();
    String entityHashStr = String.join(
      " ", declaringClassName, methodBindingStr);
    int entityHash = entityHashStr.hashCode();
    int classHash = declaringClassName.hashCode();

    // Persist constructor's parameters
    for (Object varDeclObj : node.parameters()) {
      visitConstructorParameter(
        javaConstructor, (SingleVariableDeclaration) varDeclObj, entityHashStr);
    }

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.CONSTRUCTOR, AstType.DEFINITION, entityHash);

    // Set JavaMemberType fields
    javaMemberType.setTypeHash(classHash);
    javaMemberType.setMemberTypeHash(classHash);
    javaMemberType.setKind(MemberTypeKind.CONSTRUCTOR);
    javaMemberType.setVisibility(getVisibility(node.getModifiers()));
    javaMemberType.setMemberAstNode(javaAstNode);

    // Set JavaEntity fields
    javaConstructor.setAstNodeId(javaAstNode.getId());
    javaConstructor.setEntityHash(entityHash);
    javaConstructor.setName(simpleName.toString());
    javaConstructor.setQualifiedName(declaringClassName);

    persistRow(javaMemberType);
    persistRow(javaConstructor);
  }

  private void visitMethodDeclaration(MethodDeclaration node) {
    JavaMethod javaMethod = new JavaMethod();
    JavaMemberType javaMemberType = new JavaMemberType();
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

    javaMethod.setFinal(Flags.isFinal(modifiers));
    javaMethod.setStatic(Flags.isStatic(modifiers));
    javaMethod.setTypeHash(typeHash);
    javaMethod.setQualifiedType(qualifiedType);

    // persist method's parameters
    for (Object varDeclObj : node.parameters()) {
      visitMethodParameter(
        javaMethod, (SingleVariableDeclaration) varDeclObj, entityHashStr);
    }

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.METHOD, astType, entityHash);

    // Set JavaMemberType fields
    javaMemberType.setTypeHash(classHash);
    javaMemberType.setMemberTypeHash(typeHash);
    javaMemberType.setKind(MemberTypeKind.METHOD);
    javaMemberType.setVisibility(getVisibility(node.getModifiers()));
    javaMemberType.setMemberAstNode(javaAstNode);

    // Set JavaEntity fields
    javaMethod.setAstNodeId(javaAstNode.getId());
    javaMethod.setEntityHash(entityHash);
    javaMethod.setName(simpleName.toString());
    javaMethod.setQualifiedName(qualifiedName);

    persistRow(javaMemberType);
    persistRow(javaMethod);
  }

  private void visitConstructorParameter(
    JavaConstructor javaConstructor, SingleVariableDeclaration node,
    String entityHashStrPrefix)
  {
    JavaVariable javaVariable = new JavaVariable();
    String qualifiedType = node.getType().resolveBinding().getQualifiedName();
    SimpleName simpleName = node.getName();
    String qualifiedName = simpleName.getFullyQualifiedName();
    String entityHashStr = String.join(
      " ", entityHashStrPrefix, qualifiedType, qualifiedName);
    int modifiers = node.getModifiers();
    int entityHash = entityHashStr.hashCode();
    int typeHash = qualifiedType.hashCode();

    javaVariable.setFinal(Flags.isFinal(modifiers));
    javaVariable.setStatic(Flags.isStatic(modifiers));
    javaVariable.setTypeHash(typeHash);
    javaVariable.setQualifiedType(qualifiedType);

    javaConstructor.addJavaConVarParam(javaVariable);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE,
      AstType.DECLARATION, entityHash);

    // Set JavaEntity fields
    javaVariable.setAstNodeId(javaAstNode.getId());
    javaVariable.setEntityHash(entityHash);
    javaVariable.setName(simpleName.toString());
    javaVariable.setQualifiedName(qualifiedName);
  }

  private void visitMethodParameter(
    JavaMethod javaMethod, SingleVariableDeclaration node,
    String entityHashStrPrefix)
  {
    JavaVariable javaVariable = new JavaVariable();
    String qualifiedType = node.getType().resolveBinding().getQualifiedName();
    SimpleName simpleName = node.getName();
    String qualifiedName = simpleName.getFullyQualifiedName();
    String entityHashStr = String.join(
      " ", entityHashStrPrefix, qualifiedType, qualifiedName);
    int modifiers = node.getModifiers();
    int entityHash = entityHashStr.hashCode();
    int typeHash = qualifiedType.hashCode();

    javaVariable.setFinal(Flags.isFinal(modifiers));
    javaVariable.setStatic(Flags.isStatic(modifiers));
    javaVariable.setTypeHash(typeHash);
    javaVariable.setQualifiedType(qualifiedType);

    javaMethod.addJavaMetVarParam(javaVariable);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE, AstType.DECLARATION, entityHash);

    // Set JavaEntity fields
    javaVariable.setAstNodeId(javaAstNode.getId());
    javaVariable.setEntityHash(entityHash);
    javaVariable.setName(simpleName.toString());
    javaVariable.setQualifiedName(qualifiedName);
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
    int entityHash = entityHashStr.hashCode();

    javaEnumConstant.setValue(variableBinding.getVariableId());

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.ENUM_CONSTANT, AstType.USAGE, entityHash
    );

    // Set JavaEntity fields
    javaEnumConstant.setAstNodeId(javaAstNode.getId());
    javaEnumConstant.setEntityHash(entityHash);
    javaEnumConstant.setName(name);
    javaEnumConstant.setQualifiedName(qualifiedName);

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
    AstType astType;

    if (variableBinding.isField()) {
      ITypeBinding classBinding = variableBinding.getDeclaringClass();
      String declaringClassName = classBinding.getQualifiedName();
      entityHashStr = String.join(
        " ", declaringClassName, qualifiedType, name);
    } else {
      IMethodBinding declaringMethodBinding =
        variableBinding.getVariableDeclaration().getDeclaringMethod();
      ITypeBinding classBinding = declaringMethodBinding.getDeclaringClass();
      String methodBindingStr = declaringMethodBinding.toString();
      String declaringClassName = classBinding.getQualifiedName();
      entityHashStr = String.join(
        " ", declaringClassName,
        methodBindingStr, qualifiedType, name);
    }

    int modifiers = variableBinding.getModifiers();
    int entityHash = entityHashStr.hashCode();
    int typeHash = qualifiedType.hashCode();

    javaVariable.setFinal(Flags.isFinal(modifiers));
    javaVariable.setStatic(Flags.isStatic(modifiers));
    javaVariable.setTypeHash(typeHash);
    javaVariable.setQualifiedType(qualifiedType);

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
    } else {
      astType = AstType.READ;
    }

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE, astType, entityHash);

    // Set JavaEntity fields
    javaVariable.setAstNodeId(javaAstNode.getId());
    javaVariable.setEntityHash(entityHash);
    javaVariable.setName(name);
    javaVariable.setQualifiedName(qualifiedName);

    persistRow(javaVariable);
  }

  private void visitJavaDoc(Javadoc node, int entityHash) {
    JavaDocComment javaDocComment = new JavaDocComment();
    String commentString = node.toString();

    javaDocComment.setContent(commentString);
    javaDocComment.setContentHash(commentString.hashCode());
    javaDocComment.setEntityHash(entityHash);

    persistRow(javaDocComment);
  }

  private List<JavaAstNode> queryJavaAstNodeDefinitionsByEntityHash(
    int entityHash)
  {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    cr
      .select(root)
      .where(
        cb.and(
          cb.equal(root.get("entityHash"), entityHash),
          cb.equal(root.get("astType"), AstType.DEFINITION))
      );

    return em.createQuery(cr).getResultList();
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

  private void persistRow(Object jpaObject) {
    em.getTransaction().begin();
    em.persist(jpaObject);
    em.getTransaction().commit();
  }
}
