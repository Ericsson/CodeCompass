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
import java.util.stream.Stream;

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
    IMethodBinding methodBinding =
      node.resolveConstructorBinding().getMethodDeclaration();
    String qualifiedName = node.getType().resolveBinding().getQualifiedName();
    String entityHashStr = String.join(
      " ", qualifiedName, methodBinding.toString());
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
    int entityHash = node.hashCode();

    // Persist Enum constants
    System.out.println(node.enumConstants());
    List<?> enumConstants = node.enumConstants();
    for (int i = 0; i < enumConstants.size(); i++) {
      visitEnumConstantDeclaration(javaEnum, (EnumConstantDeclaration) enumConstants.get(i), i);
    }

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.ENUM, AstType.DECLARATION, entityHash);

    // Set JavaEntity fields
    javaEnum.setAstNodeId(javaAstNode.getId());
    javaEnum.setEntityHash(entityHash);
    javaEnum.setName(node.getName().toString());
    javaEnum.setQualifiedName(node.resolveBinding().getQualifiedName());

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
  public boolean visit(FieldAccess node) {
    // System.out.println("FIELDACCESS");
    // System.out.println(node);
    // System.out.println(node.resolveFieldBinding().getType());
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
    List<?> modifiers = node.modifiers();

    for (Object varDeclFragObj : node.fragments()) {
      visitFieldDeclarationFragment(
        (VariableDeclarationFragment) varDeclFragObj,
        declaringClassName, qualifiedType, modifiers);
    }
    return super.visit(node);
  }

  @Override
  public boolean visit(ImportDeclaration node) {
    // mindkét mező egy fileid

    /*
    JavaImport _import = new JavaImport();
    _import.setImporter(...);
    _import.setImported(...);

    persistRow(_import);
    */

    return super.visit(node);
  }

  @Override
  public boolean visit(InfixExpression node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(Initializer node) {
    // System.out.println(node);
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
  public boolean visit(Javadoc node) {

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
    IMethodBinding methodBinding =
      node.resolveMethodBinding().getMethodDeclaration();
    String qualifiedType = methodBinding.getReturnType().getQualifiedName();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr = methodBinding.toString();
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
    javaMethod.setName(node.getName().toString());
    javaMethod.setQualifiedName(node.getName().getFullyQualifiedName());

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
    ITypeBinding classBinding =
      node.resolveMethodBinding().getDeclaringClass();
    IMethodBinding methodBinding =
      node.resolveMethodBinding().getMethodDeclaration();
    String qualifiedType = methodBinding.getReturnType().getQualifiedName();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr = methodBinding.toString();
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
    javaMethod.setName(node.getName().toString());
    javaMethod.setQualifiedName(node.getName().getFullyQualifiedName());

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
  public boolean visit(ParenthesizedExpression node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ProvidesDirective node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(QualifiedName node) {
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
    // System.out.println(node);
    // System.out.println(node.resolveBinding().getName());
    // System.out.println("==================================");
    return super.visit(node);
  }

  @Override
  public boolean visit(SimpleType node) {
    JavaRecord javaRecord = new JavaRecord();
    String qualifiedName = node.resolveBinding().getQualifiedName();
    int entityHash = qualifiedName.hashCode();

    javaRecord.setIsAbstract(Flags.isAbstract(node.getFlags()));

    JavaAstNode javaAstNode =
      persistJavaAstNodeRow(
        node, SymbolType.TYPE, AstType.USAGE, entityHash);

    // Set JavaEntity fields
    javaRecord.setAstNodeId(javaAstNode.getId());
    javaRecord.setEntityHash(entityHash);
    javaRecord.setName(node.getName().toString());
    javaRecord.setQualifiedName(qualifiedName);

    persistRow(javaRecord);

    return super.visit(node);
  }

  @Override
  public boolean visit(SingleMemberAnnotation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(SingleVariableDeclaration node) {
//     System.out.println(node);
//     JavaVariable javaVariable = new JavaVariable();
//     String qualifiedType = "";
//     int entityHash = node.resolveBinding().toString().hashCode();
//     int typeHash = qualifiedType.hashCode();
//
//     try {
//       qualifiedType = node.getType().resolveBinding().getQualifiedName();
//       javaVariable.setQualifiedType(qualifiedType);
//     } catch (NullPointerException ignored) {
//     }
//
//     javaVariable.setTypeHash(typeHash);
//
//     JavaAstNode javaAstNode = persistJavaAstNodeRow(
//       node, SymbolType.VARIABLE, AstType.DECLARATION, entityHash);
//
//     // Set JavaEntity fields
//     javaVariable.setAstNodeId(javaAstNode.getId());
//     javaVariable.setEntityHash(entityHash);
//     javaVariable.setName(node.getName().toString());
//     javaVariable.setQualifiedName(node.getName().getFullyQualifiedName());
//
//
//     persistRow(javaVariable);
//
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
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(TypeDeclaration node) {
    JavaRecord javaRecord = new JavaRecord();
    String qualifiedName = node.resolveBinding().getQualifiedName();
    int entityHash = qualifiedName.hashCode();

    javaRecord.setIsAbstract(Flags.isAbstract(node.getFlags()));

    JavaAstNode javaAstNode =
      persistJavaAstNodeRow(
        node, SymbolType.TYPE, AstType.DEFINITION, entityHash);

    // Set JavaEntity fields
    javaRecord.setAstNodeId(javaAstNode.getId());
    javaRecord.setEntityHash(entityHash);
    javaRecord.setName(node.getName().toString());
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

  private void visitEnumConstantDeclaration(
    JavaEnum javaEnum, EnumConstantDeclaration node, int index)
  {
    JavaEnumConstant enumConstant = new JavaEnumConstant();
    int ecnEntityHash = node.hashCode();

    enumConstant.setValue(index);
    javaEnum.addJavaEnumConstant(enumConstant);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.ENUM_CONSTANT,
      AstType.DECLARATION, ecnEntityHash);

    // Set JavaEntity fields
    enumConstant.setAstNodeId(javaAstNode.getId());
    enumConstant.setEntityHash(ecnEntityHash);
    enumConstant.setName(node.getName().toString());
    enumConstant.setQualifiedName(
      node.getName().getFullyQualifiedName());
  }

  private void visitFieldDeclarationFragment(
    VariableDeclarationFragment node,
    String declaringClassName, String qualifiedType, List<?> modifiers)
  {
    JavaVariable javaVariable = new JavaVariable();
    JavaMemberType javaMemberType = new JavaMemberType();
    String qualifiedName = node.getName().getFullyQualifiedName();
    String entityHashStr = String.join(
      " ", declaringClassName, qualifiedType, qualifiedName);
    int entityHash = entityHashStr.hashCode();
    int classHash = declaringClassName.hashCode();
    int typeHash = qualifiedType.hashCode();

    javaVariable.setTypeHash(typeHash);
    javaVariable.setQualifiedType(qualifiedType);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE, AstType.DECLARATION, entityHash);

    // Set JavaMemberType fields
    javaMemberType.setTypeHash(classHash);
    javaMemberType.setMemberTypeHash(typeHash);
    javaMemberType.setKind(MemberTypeKind.FIELD);
    javaMemberType.setVisibility(getVisibility(modifiers));
    javaMemberType.setMemberAstNode(javaAstNode);

    // Set JavaEntity fields
    javaVariable.setAstNodeId(javaAstNode.getId());
    javaVariable.setEntityHash(entityHash);
    javaVariable.setName(node.getName().toString());
    javaVariable.setQualifiedName(qualifiedName);

    persistRow(javaMemberType);
    persistRow(javaVariable);
  }

  private void visitVariableDeclarationFragment(
    VariableDeclarationFragment node, String qualifiedType)
  {
    JavaVariable javaVariable = new JavaVariable();
    String qualifiedName = node.getName().getFullyQualifiedName();
    IMethodBinding methodBinding =
      node.resolveBinding().getDeclaringMethod();
    ITypeBinding classBinding = methodBinding.getDeclaringClass();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr = methodBinding.toString();
    String methodEntityHashStr = String.join(
      " ", declaringClassName, methodBindingStr);
    String entityHashStr = String.join(
      " ",declaringClassName, methodBindingStr,
      qualifiedType, qualifiedName);
    int methodEntityHash = methodEntityHashStr.hashCode();
    int entityHash = entityHashStr.hashCode();
    int typeHash = qualifiedType.hashCode();
    List<JavaAstNode> javaAstNodeDefs =
      queryJavaAstNodeDefinitionsByEntityHash(methodEntityHash);
    JavaAstNode javaAstNodeDef = javaAstNodeDefs.get(0);

    javaVariable.setTypeHash(typeHash);
    javaVariable.setQualifiedType(qualifiedType);

    if (methodBinding.isConstructor()) {
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
      node.toString().contains("=") ? AstType.DEFINITION : AstType.DECLARATION,
      entityHash
    );

    // Set JavaEntity fields
    javaVariable.setAstNodeId(javaAstNode.getId());
    javaVariable.setEntityHash(entityHash);
    javaVariable.setName(node.getName().toString());
    javaVariable.setQualifiedName(qualifiedName);

    persistRow(javaVariable);
  }

  private void visitConstructorDeclaration(MethodDeclaration node) {
    JavaConstructor javaConstructor = new JavaConstructor();
    JavaMemberType javaMemberType = new JavaMemberType();
    ITypeBinding classBinding = node.resolveBinding().getDeclaringClass();
    IMethodBinding methodBinding = node.resolveBinding().getMethodDeclaration();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr =  methodBinding.toString();
    String entityHashStr = String.join(
      " ", declaringClassName, methodBindingStr);
    int entityHash = entityHashStr.hashCode();
    int classHash = declaringClassName.hashCode();

    // Persist constructor's parameters
    for (Object varDeclObj : node.parameters()) {
      visitConstructorParameter(
        javaConstructor, (SingleVariableDeclaration) varDeclObj);
    }

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.CONSTRUCTOR, AstType.DEFINITION, entityHash);

    // Set JavaMemberType fields
    javaMemberType.setTypeHash(classHash);
    javaMemberType.setMemberTypeHash(classHash);
    javaMemberType.setKind(MemberTypeKind.CONSTRUCTOR);
    javaMemberType.setVisibility(getVisibility(node.modifiers()));
    javaMemberType.setMemberAstNode(javaAstNode);

    // Set JavaEntity fields
    javaConstructor.setAstNodeId(javaAstNode.getId());
    javaConstructor.setEntityHash(entityHash);
    javaConstructor.setName(node.getName().toString());
    javaConstructor.setQualifiedName(declaringClassName);

    persistRow(javaMemberType);
    persistRow(javaConstructor);
  }

  private void visitMethodDeclaration(MethodDeclaration node) {
    JavaMethod javaMethod = new JavaMethod();
    JavaMemberType javaMemberType = new JavaMemberType();
    ITypeBinding classBinding = node.resolveBinding().getDeclaringClass();
    IMethodBinding methodBinding = node.resolveBinding().getMethodDeclaration();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr = methodBinding.toString();
    String qualifiedType =
      node.getReturnType2().resolveBinding().getQualifiedName();
    AstType astType =
      classBinding.isInterface() ? AstType.DECLARATION : AstType.DEFINITION;
    String entityHashStr = String.join(
      " ", declaringClassName, methodBindingStr);
    int entityHash = entityHashStr.hashCode();
    int classHash = declaringClassName.hashCode();
    int typeHash = qualifiedType.hashCode();

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
    javaMemberType.setVisibility(getVisibility(node.modifiers()));
    javaMemberType.setMemberAstNode(javaAstNode);

    // Set JavaEntity fields
    javaMethod.setAstNodeId(javaAstNode.getId());
    javaMethod.setEntityHash(entityHash);
    javaMethod.setName(node.getName().toString());
    javaMethod.setQualifiedName(node.getName().getFullyQualifiedName());

    persistRow(javaMemberType);
    persistRow(javaMethod);
  }

  private void visitConstructorParameter(
    JavaConstructor javaConstructor, SingleVariableDeclaration node)
  {
    JavaVariable javaVariable = new JavaVariable();
    String qualifiedType = node.getType().resolveBinding().getQualifiedName();
    int entityHash = node.hashCode();
    int typeHash = qualifiedType.hashCode();

    javaVariable.setTypeHash(typeHash);
    javaVariable.setQualifiedType(qualifiedType);

    javaConstructor.addJavaConVarParam(javaVariable);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE,
      AstType.DECLARATION, entityHash);

    // Set JavaEntity fields
    javaVariable.setAstNodeId(javaAstNode.getId());
    javaVariable.setEntityHash(entityHash);
    javaVariable.setName(node.getName().toString());
    javaVariable.setQualifiedName(
      node.getName().getFullyQualifiedName());
  }

  private void visitMethodParameter(
    JavaMethod javaMethod, SingleVariableDeclaration node,
    String entityHashStrPrefix)
  {
    JavaVariable javaVariable = new JavaVariable();
    String qualifiedType = node.getType().resolveBinding().getQualifiedName();
    String qualifiedName = node.getName().getFullyQualifiedName();
    String entityHashStr = String.join(
      " ", entityHashStrPrefix, qualifiedType, qualifiedName);
    int entityHash = entityHashStr.hashCode();
    int typeHash = qualifiedType.hashCode();

    javaVariable.setTypeHash(typeHash);
    javaVariable.setQualifiedType(qualifiedType);

    javaMethod.addJavaMetVarParam(javaVariable);

    JavaAstNode javaAstNode = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE, AstType.DECLARATION, entityHash);

    // Set JavaEntity fields
    javaVariable.setAstNodeId(javaAstNode.getId());
    javaVariable.setEntityHash(entityHash);
    javaVariable.setName(node.getName().toString());
    javaVariable.setQualifiedName(qualifiedName);
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

  private Visibility getVisibility(List<?> modifiers) {
    for (Object mod : modifiers) {
      Optional<Visibility> visibility =
        Stream.of(Visibility.values())
          .filter(v -> mod.toString().equals(v.getName()))
          .findFirst();

      if (visibility.isPresent()) {
        return visibility.get();
      }
    }

    return Visibility.PACKAGE_PRIVATE;
  }

  private void persistRow(Object jpaObject) {
    em.getTransaction().begin();
    em.persist(jpaObject);
    em.getTransaction().commit();
  }
}
