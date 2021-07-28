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
  public boolean visit(BooleanLiteral node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(BreakStatement node) {
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
  public boolean visit(CharacterLiteral node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ClassInstanceCreation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(CompilationUnit node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ConditionalExpression node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ConstructorInvocation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ContinueStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(CreationReference node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(Dimension node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(DoStatement node) {
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
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(EnumConstantDeclaration node) {
    /*
    JavaEnumConstant enumConstant = new JavaEnumConstant();
    enumConstant.setValue(node.getFlags());
    enumConstant.setTypeId(node.getNodeType());
    enumConstant.setQualifiedName(node.getName().toString());
    em.persist(enumConstant);
    em.getTransaction().commit();
    System.out.println(node);
    */
    return super.visit(node);
  }

  @Override
  public boolean visit(EnumDeclaration node) {
    /*
    JavaEnum _enum = new JavaEnum();
    _enum.setName(node.getName().toString());
    _enum.setTypeId(node.getNodeType());
    em.persist(_enum);
    em.getTransaction().commit();
    */
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

    JavaImport _import = new JavaImport();
    // _import.setImporter(...);
    _import.setImported(node.getFlags());

    persistRow(_import);
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
  public boolean visit(LineComment node) {
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
      String typeString = node.getReturnType2().toString();
      // megnézni, hogy qualified-e, vagy nem, vagy egyszer ez, egyszer az
      // valószínóleg nem, de ez kéne

      persistJavaAstNodeRow(node, SymbolType.METHOD, AstType.DECLARATION);

      setJavaEntityFields(javaMethod, node);
      javaMethod.setTypeHash(typeString.hashCode());
      javaMethod.setQualifiedType(typeString);

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
  public boolean visit(NullLiteral node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(NumberLiteral node) {
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
  public boolean visit(PostfixExpression node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(PrefixExpression node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(PrimitiveType node) {
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
  public boolean visit(RequiresDirective node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(ReturnStatement node) {
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
    String typeString = node.getType().toString();

    persistJavaAstNodeRow(node, SymbolType.VARIABLE, AstType.DECLARATION);

    setJavaEntityFields(javaVariable, node);
    javaVariable.setTypeHash(typeString.hashCode());
    javaVariable.setQualifiedType(typeString);

    persistRow(javaVariable);

    return super.visit(node);
  }

  @Override
  public boolean visit(StringLiteral node) {
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
  public boolean visit(SwitchCase node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(SwitchStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(SynchronizedStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(TagElement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(TextElement node) {
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
  public boolean visit(TryStatement node) {
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
  public boolean visit(UnionType node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(UsesDirective node) {
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

  @Override
  public boolean visit(WhileStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(WildcardType node) {
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
    PositionInfo positionInfo = new PositionInfo(this.cu, node);

    // location_file = file id a File táblából

    // symbol_type, ast_type = enum, megmondja, hogy milyen típusú a kifejezés (értékadás, def. stb..)
    // symbol type = az adott identifier micsoda? változó, enum, enumkonstans, stb

    // visibleinsourcecode: akkor lesz false, ha az adott függvényt, vagy akármit
    // nem közvetlenül hívjuk a forráskódból, hanem hívunk valamit egy libraryből, ami meghívja aztán ezt
    try {
      Method getJavadocMethod =
              node.getClass().getMethod("getJavadoc", (Class<?>[]) null);
      Javadoc javadoc =
              (Javadoc) getJavadocMethod.invoke(node, (Object[]) null);

      javaAstNode.setAstValue(
              node.toString().substring(javadoc.toString().length())
      );
    } catch (NoSuchMethodException | NullPointerException e) {
      javaAstNode.setAstValue(node.toString());
    } catch (IllegalAccessException | InvocationTargetException e) {
      e.printStackTrace();
    }

    javaAstNode.setLocation_range_start_line(positionInfo.getStartLine());
    javaAstNode.setLocation_range_start_column(positionInfo.getStartColumn());
    javaAstNode.setLocation_range_end_line(positionInfo.getEndLine());
    javaAstNode.setLocation_range_end_column(positionInfo.getEndColumn());
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
