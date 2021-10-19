package parser.srcjava;

import model.*;
import model.enums.AstType;
import model.enums.SymbolType;
import org.eclipse.jdt.core.Flags;
import org.eclipse.jdt.core.dom.*;

import javax.persistence.EntityManager;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class AstVisitor extends ASTVisitor {

  private final CompilationUnit cu;
  private final EntityManager em;
  private final long fileId;

  public AstVisitor(CompilationUnit cu, EntityManager em, long fileId) {
    this.cu = cu;
    this.em = em;
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
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(Block node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(BlockComment node) {
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
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ClassInstanceCreation node) {
    JavaConstructor javaConstructor = new JavaConstructor();
    String qualifiedName = node.getType().resolveBinding().getQualifiedName();
    int hashCode = (
      qualifiedName +
      node.resolveConstructorBinding().getMethodDeclaration().toString()
    ).hashCode();

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
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(FieldDeclaration node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ForStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(IfStatement node) {
    // System.out.println(node);
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
      int hashCode =
        ((MethodDeclaration) parent)
          .resolveBinding().getMethodDeclaration().toString().hashCode();

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

    // Method
    if (!node.isConstructor()) {
      JavaMethod javaMethod = new JavaMethod();
      String qTypeString = "";
      int hashCode =
        node.resolveBinding().getMethodDeclaration().toString().hashCode();


      try {
        qTypeString = node.getReturnType2().resolveBinding().getQualifiedName();
      } catch (NullPointerException ignored) {
      }

      javaMethod.setTypeHash(qTypeString.hashCode());
      javaMethod.setQualifiedType(qTypeString);

      // persist method's parameters
      for (Object svdObj : node.parameters()) {
        SingleVariableDeclaration variableDeclarationNode =
          ((SingleVariableDeclaration) svdObj);
        JavaVariable javaVariable = new JavaVariable();
        int svdHashCode = variableDeclarationNode.hashCode();

        javaVariable.setQualifiedType(
          variableDeclarationNode.getType().resolveBinding().getQualifiedName()
        );

        javaMethod.addJavaMetVarParam(javaVariable);

        long javaAstNodeId = persistJavaAstNodeRow(
          variableDeclarationNode, SymbolType.VARIABLE,
          AstType.DECLARATION, svdHashCode);

        //Set JavaEntity fields
        javaVariable.setAstNodeId(javaAstNodeId);
        javaVariable.setEntityHash(hashCode);
        javaVariable.setName(variableDeclarationNode.getName().toString());
        javaVariable.setQualifiedName(
          variableDeclarationNode.getName().getFullyQualifiedName());
      }

      long javaAstNodeId = persistJavaAstNodeRow(
        node, SymbolType.METHOD, AstType.DECLARATION, hashCode);

      // Set JavaEntity fields
      javaMethod.setAstNodeId(javaAstNodeId);
      javaMethod.setEntityHash(hashCode);
      javaMethod.setName(node.getName().toString());
      javaMethod.setQualifiedName(node.getName().getFullyQualifiedName());

      persistRow(javaMethod);

    // Constructor
    } else {
      JavaConstructor javaConstructor = new JavaConstructor();
      ASTNode parent = node.getParent();
      String qualifiedName = "";

      if (parent instanceof TypeDeclaration) {
        qualifiedName =
          ((TypeDeclaration) parent).resolveBinding().getQualifiedName();
      } else if (parent instanceof EnumDeclaration) {
        qualifiedName =
          ((EnumDeclaration) parent).resolveBinding().getQualifiedName();
      }

      int hashCode = (
        qualifiedName +
          node.resolveBinding().getMethodDeclaration().toString()
      ).hashCode();

      // Persist constructor's parameters
      for (Object svdObj : node.parameters()) {
        SingleVariableDeclaration variableDeclarationNode =
          ((SingleVariableDeclaration) svdObj);
        JavaVariable javaVariable = new JavaVariable();
        int svdHashCode = variableDeclarationNode.hashCode();

        javaVariable.setQualifiedType(
          variableDeclarationNode.getType().resolveBinding().getQualifiedName()
        );

        javaConstructor.addJavaConVarParam(javaVariable);

        long javaAstNodeId = persistJavaAstNodeRow(
          variableDeclarationNode, SymbolType.VARIABLE,
          AstType.DECLARATION, svdHashCode);

        // Set JavaEntity fields
        javaVariable.setAstNodeId(javaAstNodeId);
        javaVariable.setEntityHash(svdHashCode);
        javaVariable.setName(variableDeclarationNode.getName().toString());
        javaVariable.setQualifiedName(
          variableDeclarationNode.getName().getFullyQualifiedName());
      }

      long javaAstNodeId = persistJavaAstNodeRow(
        node, SymbolType.CONSTRUCTOR, AstType.DECLARATION, hashCode);

      // Set JavaEntity fields
      javaConstructor.setAstNodeId(javaAstNodeId);
      javaConstructor.setEntityHash(hashCode);
      javaConstructor.setName(node.getName().toString());
      javaConstructor.setQualifiedName(qualifiedName);

      persistRow(javaConstructor);
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(MethodInvocation node) {
    JavaMethod javaMethod = new JavaMethod();
    String qTypeString = "";
    int hashCode =
      node.resolveMethodBinding().getMethodDeclaration().toString().hashCode();

    try {
      qTypeString =
        node
          .resolveMethodBinding()
          .getMethodDeclaration()
          .getReturnType()
          .getQualifiedName();
    } catch (NullPointerException ignored) {
    }

    javaMethod.setTypeHash(qTypeString.hashCode());
    javaMethod.setQualifiedType(qTypeString);

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
    int hashCode = node.resolveBinding().getQualifiedName().hashCode();

    javaRecord.setIsAbstract(Flags.isAbstract(node.getFlags()));

    long javaAstNodeId =
      persistJavaAstNodeRow(
        node, SymbolType.TYPE, AstType.USAGE, hashCode);

    // Set JavaEntity fields
    javaRecord.setAstNodeId(javaAstNodeId);
    javaRecord.setEntityHash(hashCode);
    javaRecord.setName(node.getName().toString());
    javaRecord.setQualifiedName(node.resolveBinding().getQualifiedName());

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
//     JavaVariable javaVariable = new JavaVariable();
//     String qTypeString = "";
//     int hashCode = node.resolveBinding().toString().hashCode();
//
//     try {
//       qTypeString = node.getType().resolveBinding().getQualifiedName();
//       javaVariable.setQualifiedType(qTypeString);
//     } catch (NullPointerException ignored) {
//     }
//
//     javaVariable.setTypeHash(qTypeString.hashCode());
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
  public boolean visit(ThrowStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(TypeDeclaration node) {
    // Class declaration
    // System.out.println(node);
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
    // System.out.println(node);
    return super.visit(node);
  }

  private <T extends ASTNode> long persistJavaAstNodeRow(
    T node, SymbolType symbolType, AstType astType, int hashCode
  ) {
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
