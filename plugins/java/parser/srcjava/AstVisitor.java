package parser.srcjava;

import org.eclipse.jdt.core.dom.*;

import javax.persistence.EntityManager;

public class AstVisitor extends ASTVisitor {
  private final PersistManager pm;

  public AstVisitor(CompilationUnit cu, EntityManager em, long fileId) {
    this.pm = new PersistManager(cu, em, fileId);
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
    pm.persistLocalVarDeclaration(node.getException());

    return super.visit(node);
  }

  @Override
  public boolean visit(ClassInstanceCreation node) {
    pm.persistConstructorUsage(node, node.resolveConstructorBinding());

    return super.visit(node);
  }

  @Override
  public boolean visit(EnhancedForStatement node) {
    pm.persistLocalVarDeclaration(node.getParameter());

    return super.visit(node);
  }

  @Override
  public boolean visit(EnumDeclaration node) {
    pm.persistEnumDeclaration(node);

    return super.visit(node);
  }

  @Override
  public boolean visit(ExpressionMethodReference node) {
    pm.persistMethodUsage(node, node.resolveMethodBinding(), node.getName());

    return super.visit(node);
  }

  @Override
  public boolean visit(FieldAccess node) {
    pm.persistVariableUsage(node, node.resolveFieldBinding());

    return super.visit(node);
  }

  @Override
  public boolean visit(FieldDeclaration node) {
    pm.persistFieldDeclaration(node);

    return super.visit(node);
  }

  @Override
  public boolean visit(ImportDeclaration node) {
    // mindkét mező egy fileid (vagy nem)
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(Initializer node) {
    pm.persistInitializer(node);

    return super.visit(node);
  }

  @Override
  public boolean visit(LambdaExpression node) {
    pm.persistLambdaExpression(node);

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
      pm.persistConstructorDeclaration(node);
    } else {
      pm.persistMethodDeclaration(node);
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(MethodInvocation node) {
    pm.persistMethodUsage(node, node.resolveMethodBinding(), node.getName());

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
        pm.persistEnumConstantUsage(node, variableBinding);
      } else {
        pm.persistVariableUsage(node, variableBinding);
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
        pm.persistEnumConstantUsage(node, variableBinding);
      } else {
        pm.persistVariableUsage(node, variableBinding);
      }
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(SimpleType node) {
    pm.persistTypeUsage(node);

    return super.visit(node);
  }

  @Override
  public boolean visit(SingleMemberAnnotation node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(SuperConstructorInvocation node) {
    pm.persistConstructorUsage(node, node.resolveConstructorBinding());

    return super.visit(node);
  }

  @Override
  public boolean visit(SuperFieldAccess node) {
    pm.persistVariableUsage(node, node.resolveFieldBinding());

    return super.visit(node);
  }

  @Override
  public boolean visit(SuperMethodInvocation node) {
    pm.persistMethodUsage(node, node.resolveMethodBinding(), node.getName());

    return super.visit(node);
  }

  @Override
  public boolean visit(SuperMethodReference node) {
    pm.persistMethodUsage(node, node.resolveMethodBinding(), node.getName());

    return super.visit(node);
  }

  @Override
  public boolean visit(ThisExpression node) {
    // System.out.println(node);
    return super.visit(node);
  }

  @Override
  public boolean visit(TypeDeclaration node) {
    pm.persistTypeDeclaration(node);

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
      pm.persistLocalVarDeclaration(node);
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(VariableDeclarationStatement node) {
    // System.out.println(node);
    return super.visit(node);
  }
}
