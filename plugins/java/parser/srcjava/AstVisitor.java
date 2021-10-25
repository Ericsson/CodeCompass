package parser.srcjava;

import model.*;
import model.enums.AstType;
import model.enums.SymbolType;
import org.eclipse.jdt.core.Flags;
import org.eclipse.jdt.core.dom.*;

import javax.persistence.EntityManager;
import javax.persistence.criteria.CriteriaBuilder;
import javax.persistence.criteria.CriteriaQuery;
import javax.persistence.criteria.Root;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;

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
    // System.out.println(node.ge);
    // System.out.println(node.getLeftHandSide());
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
    String hashCodeStr = String.join(
      " ", qualifiedName, methodBinding.toString());
    int hashCode = hashCodeStr.hashCode();

    long javaAstNodeId = persistJavaAstNodeRow(
      node, SymbolType.CONSTRUCTOR, AstType.USAGE, hashCode);

    // Set JavaEntity fields
    javaConstructor.setAstNodeId(javaAstNodeId);
    javaConstructor.setEntityHash(hashCode);
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
    JavaEnum _enum = new JavaEnum();
    int hashCode = node.hashCode();

    // Persist Enum constants
    for (Object ecnObj : node.enumConstants()) {
      EnumConstantDeclaration enumConstantNode =
        ((EnumConstantDeclaration) ecnObj);
      JavaEnumConstant enumConstant = new JavaEnumConstant();
      int ecnHashCode = enumConstantNode.hashCode();

      enumConstant.setValue(enumConstantNode.getName().toString());
      _enum.addJavaEnumConstant(enumConstant);

      long javaAstNodeId = persistJavaAstNodeRow(
        enumConstantNode, SymbolType.ENUM_CONSTANT,
        AstType.DECLARATION, ecnHashCode);

      // Set JavaEntity fields
      enumConstant.setAstNodeId(javaAstNodeId);
      enumConstant.setEntityHash(ecnHashCode);
      enumConstant.setName(enumConstantNode.getName().toString());
      enumConstant.setQualifiedName(
        enumConstantNode.getName().getFullyQualifiedName());
    }

    long javaAstNodeId = persistJavaAstNodeRow(
      node, SymbolType.ENUM, AstType.DECLARATION, hashCode);

    // Set JavaEntity fields
    _enum.setAstNodeId(javaAstNodeId);
    _enum.setEntityHash(hashCode);
    _enum.setName(node.getName().toString());
    _enum.setQualifiedName(node.resolveBinding().getQualifiedName());

    persistRow(_enum);

    return super.visit(node);
  }

  @Override
  public boolean visit(ExportsDirective node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ExpressionMethodReference node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ExpressionStatement node) {
    // System.out.println(node);
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

    for (Object varDeclFragObj : node.fragments()) {
      visitFieldDeclarationFragment(
        (VariableDeclarationFragment) varDeclFragObj,
        declaringClassName, qualifiedType);
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
    JavaDocComment javaDocComment = new JavaDocComment();
    String commentString = node.toString();
    ASTNode parent = node.getParent();

    javaDocComment.setContent(commentString);
    javaDocComment.setContentHash(commentString.hashCode());
    javaDocComment.setEntityHash(node.getParent().hashCode());

    if (parent instanceof MethodDeclaration) {
      IMethodBinding methodBinding =
        ((MethodDeclaration) parent).resolveBinding().getMethodDeclaration();
      int hashCode = methodBinding.toString().hashCode();

      javaDocComment.setEntityHash(hashCode);
    }

    persistRow(javaDocComment);

    return super.visit(node);
  }

  @Override
  public boolean visit(LabeledStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(LambdaExpression node) {
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
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(MemberValuePair node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(MethodDeclaration node) {
    if (!node.isConstructor()) {
      // Method
      visitMethodDeclaration(node);

    } else {
      // Constructor
      visitConstructorDeclaration(node);
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
    String hashCodeStr = String.join(
      " ", declaringClassName, methodBindingStr);
    int hashCode = hashCodeStr.hashCode();

    javaMethod.setTypeHash(qualifiedType.hashCode());
    javaMethod.setQualifiedType(qualifiedType);

    long javaAstNodeId = persistJavaAstNodeRow(
      node, SymbolType.METHOD, AstType.USAGE, hashCode);

    // Set JavaEntity fields
    javaMethod.setAstNodeId(javaAstNodeId);
    javaMethod.setEntityHash(hashCode);
    javaMethod.setName(node.getName().toString());
    javaMethod.setQualifiedName(node.getName().getFullyQualifiedName());

    persistRow(javaMethod);

    return super.visit(node);
  }

  @Override
  public boolean visit(MethodRef node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(MethodRefParameter node) {
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
    return super.visit(node);
  }

  @Override
  public boolean visit(SimpleType node) {
    JavaRecord javaRecord = new JavaRecord();
    String qualifiedName = node.resolveBinding().getQualifiedName();
    int hashCode = qualifiedName.hashCode();

    javaRecord.setIsAbstract(Flags.isAbstract(node.getFlags()));

    long javaAstNodeId =
      persistJavaAstNodeRow(
        node, SymbolType.TYPE, AstType.USAGE, hashCode);

    // Set JavaEntity fields
    javaRecord.setAstNodeId(javaAstNodeId);
    javaRecord.setEntityHash(hashCode);
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
//     int hashCode = node.resolveBinding().toString().hashCode();
//
//     try {
//       qualifiedType = node.getType().resolveBinding().getQualifiedName();
//       javaVariable.setQualifiedType(qualifiedType);
//     } catch (NullPointerException ignored) {
//     }
//
//     javaVariable.setTypeHash(qualifiedType.hashCode());
//
//     long javaAstNodeId = persistJavaAstNodeRow(
//       node, SymbolType.VARIABLE, AstType.DECLARATION, hashCode);
//
//     // Set JavaEntity fields
//     javaVariable.setAstNodeId(javaAstNodeId);
//     javaVariable.setEntityHash(hashCode);
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
    int hashCode = qualifiedName.hashCode();

    javaRecord.setIsAbstract(Flags.isAbstract(node.getFlags()));

    long javaAstNodeId =
      persistJavaAstNodeRow(
        node, SymbolType.TYPE, AstType.DEFINITION, hashCode);

    // Set JavaEntity fields
    javaRecord.setAstNodeId(javaAstNodeId);
    javaRecord.setEntityHash(hashCode);
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

  private void visitFieldDeclarationFragment(
    VariableDeclarationFragment node,
    String declaringClassName, String qualifiedType)
  {
    JavaVariable javaVariable = new JavaVariable();
    String qualifiedName = node.getName().getFullyQualifiedName();
    String hashCodeStr = String.join(
      " ", declaringClassName, qualifiedType, qualifiedName);
    int hashCode = hashCodeStr.hashCode();

    javaVariable.setTypeHash(qualifiedType.hashCode());
    javaVariable.setQualifiedType(qualifiedType);

    long javaAstNodeId = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE, AstType.DECLARATION, hashCode);

    javaVariable.setAstNodeId(javaAstNodeId);
    javaVariable.setEntityHash(hashCode);
    javaVariable.setName(node.getName().toString());
    javaVariable.setQualifiedName(qualifiedName);

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
    String methodHashCodeStr = String.join(
      " ", declaringClassName, methodBindingStr);
    String hashCodeStr = String.join(
      " ",declaringClassName, methodBindingStr,
      qualifiedType, qualifiedName);
    int methodHashCode = methodHashCodeStr.hashCode();
    int hashCode = hashCodeStr.hashCode();
    List<JavaAstNode> javaAstNodes =
      queryJavaAstNodeDefinitionsByEntityHash(methodHashCode);
    JavaAstNode javaAstNode = javaAstNodes.get(0);

    javaVariable.setTypeHash(qualifiedType.hashCode());
    javaVariable.setQualifiedType(qualifiedType);

    if (methodBinding.isConstructor()) {
      CriteriaQuery<JavaConstructor> cr = cb.createQuery(JavaConstructor.class);
      Root<JavaConstructor> root = cr.from(JavaConstructor.class);

      JavaConstructor javaConstructor =
        queryJavaEntityByAstNodeId(cr, root, javaAstNode.getId());

      javaConstructor.addJavaConVarLocal(javaVariable);

    } else {
      CriteriaQuery<JavaMethod> cr = cb.createQuery(JavaMethod.class);
      Root<JavaMethod> root = cr.from(JavaMethod.class);

      JavaMethod javaMethod =
        queryJavaEntityByAstNodeId(cr, root, javaAstNode.getId());

      javaMethod.addJavaMetVarLocal(javaVariable);
    }

    long javaAstNodeId = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE,
      node.toString().contains("=") ? AstType.DEFINITION : AstType.DECLARATION,
      hashCode
    );

    // Set JavaEntity fields
    javaVariable.setAstNodeId(javaAstNodeId);
    javaVariable.setEntityHash(hashCode);
    javaVariable.setName(node.getName().toString());
    javaVariable.setQualifiedName(qualifiedName);

    persistRow(javaVariable);
  }

  private void visitMethodDeclaration(MethodDeclaration node) {
    JavaMethod javaMethod = new JavaMethod();
    ITypeBinding classBinding = node.resolveBinding().getDeclaringClass();
    IMethodBinding methodBinding = node.resolveBinding().getMethodDeclaration();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr = methodBinding.toString();
    String qualifiedType =
      node.getReturnType2().resolveBinding().getQualifiedName();
    AstType astType =
      classBinding.isInterface() ? AstType.DECLARATION : AstType.DEFINITION;
    String hashCodeStr = String.join(
      " ", declaringClassName, methodBindingStr);
    int hashCode = hashCodeStr.hashCode();

    javaMethod.setTypeHash(qualifiedType.hashCode());
    javaMethod.setQualifiedType(qualifiedType);

    // persist method's parameters
    for (Object varDeclObj : node.parameters()) {
      visitMethodParameter(
        javaMethod, (SingleVariableDeclaration) varDeclObj, hashCodeStr);
    }

    long javaAstNodeId = persistJavaAstNodeRow(
      node, SymbolType.METHOD, astType, hashCode);

    // Set JavaEntity fields
    javaMethod.setAstNodeId(javaAstNodeId);
    javaMethod.setEntityHash(hashCode);
    javaMethod.setName(node.getName().toString());
    javaMethod.setQualifiedName(node.getName().getFullyQualifiedName());

    persistRow(javaMethod);
  }

  private void visitConstructorDeclaration(MethodDeclaration node) {
    JavaConstructor javaConstructor = new JavaConstructor();
    ITypeBinding classBinding = node.resolveBinding().getDeclaringClass();
    IMethodBinding methodBinding = node.resolveBinding().getMethodDeclaration();
    String declaringClassName = classBinding.getQualifiedName();
    String methodBindingStr =  methodBinding.toString();
    String hashCodeStr = String.join(
      " ", declaringClassName, methodBindingStr);
    int hashCode = hashCodeStr.hashCode();

    // Persist constructor's parameters
    for (Object varDeclObj : node.parameters()) {
      visitConstructorParameter(
        javaConstructor, (SingleVariableDeclaration) varDeclObj);
    }

    long javaAstNodeId = persistJavaAstNodeRow(
      node, SymbolType.CONSTRUCTOR, AstType.DEFINITION, hashCode);

    // Set JavaEntity fields
    javaConstructor.setAstNodeId(javaAstNodeId);
    javaConstructor.setEntityHash(hashCode);
    javaConstructor.setName(node.getName().toString());
    javaConstructor.setQualifiedName(declaringClassName);

    persistRow(javaConstructor);
  }

  private void visitMethodParameter(
    JavaMethod javaMethod, SingleVariableDeclaration node,
    String hashCodeStrPrefix)
  {
    JavaVariable javaVariable = new JavaVariable();
    String qualifiedType = node.getType().resolveBinding().getQualifiedName();
    String qualifiedName = node.getName().getFullyQualifiedName();
    String hashCodeStr = String.join(
      " ", hashCodeStrPrefix, qualifiedType, qualifiedName);
    int hashCode = hashCodeStr.hashCode();

    javaVariable.setTypeHash(qualifiedType.hashCode());
    javaVariable.setQualifiedType(qualifiedType);

    javaMethod.addJavaMetVarParam(javaVariable);

    long javaAstNodeId = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE, AstType.DECLARATION, hashCode);

    // Set JavaEntity fields
    javaVariable.setAstNodeId(javaAstNodeId);
    javaVariable.setEntityHash(hashCode);
    javaVariable.setName(node.getName().toString());
    javaVariable.setQualifiedName(qualifiedName);
  }

  private void visitConstructorParameter(
    JavaConstructor javaConstructor, SingleVariableDeclaration node)
  {
    JavaVariable javaVariable = new JavaVariable();
    String qualifiedType = node.getType().resolveBinding().getQualifiedName();
    int hashCode = node.hashCode();

    javaVariable.setTypeHash(qualifiedType.hashCode());
    javaVariable.setQualifiedType(qualifiedType);

    javaConstructor.addJavaConVarParam(javaVariable);

    long javaAstNodeId = persistJavaAstNodeRow(
      node, SymbolType.VARIABLE,
      AstType.DECLARATION, hashCode);

    // Set JavaEntity fields
    javaVariable.setAstNodeId(javaAstNodeId);
    javaVariable.setEntityHash(hashCode);
    javaVariable.setName(node.getName().toString());
    javaVariable.setQualifiedName(
      node.getName().getFullyQualifiedName());
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

  private <T extends ASTNode> long persistJavaAstNodeRow(
    T node, SymbolType symbolType, AstType astType, int hashCode)
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
    javaAstNode.setEntityHash(hashCode);
    javaAstNode.setSymbolType(symbolType);
    javaAstNode.setAstType(astType);
    javaAstNode.setVisibleInSourceCode(true);

    persistRow(javaAstNode);

    return javaAstNode.getId();
  }

  private void persistRow(Object jpaObject) {
    em.getTransaction().begin();
    em.persist(jpaObject);
    em.getTransaction().commit();
  }
}
