package parser.srcjava;

import model.*;
import model.enums.AstType;
import model.enums.SymbolType;
import org.eclipse.jdt.core.dom.*;

import javax.persistence.EntityManager;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class AstVisitor extends ASTVisitor {

  private final CompilationUnit cu;
  private final EntityManager em;

  public AstVisitor(CompilationUnit cu, EntityManager em) {
    this.cu = cu;
    this.em = em;
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
    // System.out.println(node);
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

    setJavaEntityFields(_enum, node);

    for (Object ecnObj : node.enumConstants()) {
      EnumConstantDeclaration enumConstantNode =
              ((EnumConstantDeclaration) ecnObj);
      JavaEnumConstant enumConstant = new JavaEnumConstant();

      setJavaEntityFields(enumConstant, node);

      enumConstant.setValue(enumConstantNode.getName().toString());
      _enum.addJavaEnumConstant(enumConstant);

      persistJavaAstNodeRow(
              enumConstantNode, SymbolType.ENUM_CONSTANT, AstType.DECLARATION
      );
    }

    persistJavaAstNodeRow(node, SymbolType.ENUM, AstType.DECLARATION);
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
      JavaMethod javaMethod = new JavaMethod();
      String qTypeString = "";
      try {
        qTypeString = node.getReturnType2().resolveBinding().getQualifiedName();
      } catch (NullPointerException ignored) {
        // System.out.println(node.getName());
      }

      // System.out.println(node.resolveBinding());
      // System.out.println(node.getName().resolveBinding());
      // megnézni, hogy qualified-e, vagy nem, vagy egyszer ez, egyszer az
      // valószínóleg nem, de ez kéne

      persistJavaAstNodeRow(node, SymbolType.METHOD, AstType.DECLARATION);

      setJavaEntityFields(javaMethod, node);
      javaMethod.setTypeHash(qTypeString.hashCode());
      javaMethod.setQualifiedType(qTypeString);

      persistRow(javaMethod);
    } else {
      JavaConstructor javaConstructor = new JavaConstructor();

      persistJavaAstNodeRow(node, SymbolType.CONSTRUCTOR, AstType.DECLARATION);
      setJavaEntityFields(javaConstructor, node);
      persistRow(javaConstructor);
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(MethodInvocation node) {
    // System.out.println(node);
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
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(SingleMemberAnnotation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(SingleVariableDeclaration node) {
    JavaVariable javaVariable = new JavaVariable();
    String qTypeString = "";

    try {
      qTypeString = node.getType().resolveBinding().getQualifiedName();
      javaVariable.setQualifiedType(qTypeString);
    } catch (NullPointerException ignored) {
      // System.out.println(node.getName());
    }

    persistJavaAstNodeRow(node, SymbolType.VARIABLE, AstType.DECLARATION);

    setJavaEntityFields(javaVariable, node);
    javaVariable.setTypeHash(qTypeString.hashCode());

    persistRow(javaVariable);

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

  private void setJavaEntityFields(JavaEntity javaEntity, ASTNode node) {
    try {
      Method getNameMethod =
              node.getClass().getMethod("getName", (Class<?>[]) null);
      Name name =
              (Name) getNameMethod.invoke(node, (Object[]) null);

      // javaEntity.setAstNodeId(...);
      javaEntity.setEntityHash(node.hashCode());
      javaEntity.setName(name.toString());

      // resolveBindings ?
      javaEntity.setQualifiedName(name.getFullyQualifiedName());
    } catch (NoSuchMethodException e) {
      e.printStackTrace();
    } catch (IllegalAccessException e) {
      e.printStackTrace();
    } catch (InvocationTargetException e) {
      e.printStackTrace();
    }
  }

  private void persistJavaAstNodeRow(
    ASTNode node, SymbolType symbolType, AstType astType
  ) {
    JavaAstNode javaAstNode = new JavaAstNode();
    PositionInfo positionInfo;

    // location_file = file id a File táblából
    // visibleinsourcecode: akkor lesz false, ha az adott függvényt, vagy akármit
    // nem közvetlenül hívjuk a forráskódból, hanem hívunk valamit egy libraryből, ami meghívja aztán ezt
    try {
      Method getJavadocMethod =
              node.getClass().getMethod("getJavadoc", (Class<?>[]) null);
      Javadoc javadoc =
              (Javadoc) getJavadocMethod.invoke(node, (Object[]) null);

      int javadocLen = javadoc.toString().length();
      positionInfo = new PositionInfo(this.cu, node, javadocLen);

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
    javaAstNode.setEntityHash(node.hashCode());
    javaAstNode.setSymbolType(symbolType);
    javaAstNode.setAstType(astType);

    persistRow(javaAstNode);
  }

  private void persistRow(Object jpaObject) {
    em.getTransaction().begin();
    em.persist(jpaObject);
    em.getTransaction().commit();
  }
}
