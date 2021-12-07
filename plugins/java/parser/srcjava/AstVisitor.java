package parser.srcjava;

import org.eclipse.jdt.core.dom.*;

import javax.persistence.EntityManager;

import java.util.logging.Level;

import static logger.Logger.LOGGER;

public class AstVisitor extends ASTVisitor {
  private final String fileCounterStr;
  private final CompilationUnit cu;
  private final PersistManager pm;
  private boolean errorDueParsing;

  public AstVisitor(
    CompilationUnit cu, EntityManager em, long fileId, String fileCounterStr)
  {
    this.fileCounterStr = fileCounterStr;
    this.cu = cu;
    this.pm = new PersistManager(cu, em, fileId);
    this.errorDueParsing = false;
  }

  @Override
  public boolean visit(CatchClause node) {
    try {
      pm.persistLocalVarDeclaration(node.getException());
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(ClassInstanceCreation node) {
    try {
      pm.persistConstructorUsage(node, node.resolveConstructorBinding());
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(EnhancedForStatement node) {
    try {
      pm.persistLocalVarDeclaration(node.getParameter());
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(EnumDeclaration node) {
    try {
      pm.persistEnumDeclaration(node);
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(ExpressionMethodReference node) {
    try {
      pm.persistMethodUsage(node, node.resolveMethodBinding(), node.getName());
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(FieldAccess node) {
    try {
      pm.persistVariableUsage(node, node.resolveFieldBinding());
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(FieldDeclaration node) {
    try {
      pm.persistFieldDeclaration(node);
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(Initializer node) {
    try {
      pm.persistInitializer(node);
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(LambdaExpression node) {
    try {
      pm.persistLambdaExpression(node);
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(MethodDeclaration node) {
    try {
      if (node.isConstructor()) {
        pm.persistConstructorDeclaration(node);
      } else {
        pm.persistMethodDeclaration(node);
      }
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(MethodInvocation node) {
    try {
      pm.persistMethodUsage(node, node.resolveMethodBinding(), node.getName());
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(QualifiedName node) {
    try {
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
        } else if (variableBinding.getDeclaringClass() != null){
          pm.persistVariableUsage(node, variableBinding);
        }
      }
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(SimpleName node) {
    try {
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
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(SimpleType node) {
    try {
      pm.persistTypeUsage(node);
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(SuperConstructorInvocation node) {
    try {
      pm.persistConstructorUsage(node, node.resolveConstructorBinding());
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(SuperFieldAccess node) {
    try {
      pm.persistVariableUsage(node, node.resolveFieldBinding());
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(SuperMethodInvocation node) {
    try {
      pm.persistMethodUsage(node, node.resolveMethodBinding(), node.getName());
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(SuperMethodReference node) {
    try {
      pm.persistMethodUsage(node, node.resolveMethodBinding(), node.getName());
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(TypeDeclaration node) {
    try {
      pm.persistTypeDeclaration(node);
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  @Override
  public boolean visit(VariableDeclarationFragment node) {
    try {
      IVariableBinding nodeBinding = node.resolveBinding();
      ASTNode parent = node.getParent();

      if (!nodeBinding.isField() && !(parent instanceof LambdaExpression)) {
        pm.persistLocalVarDeclaration(node);
      }
    } catch (Exception ex) {
      printErrorDuringParsing(node);
      errorDueParsing = true;
    }

    return super.visit(node);
  }

  private void printErrorDuringParsing(ASTNode node) {
    int startPosition = node.getStartPosition();
    int startLine = cu.getLineNumber(startPosition);
    int startColumn = cu.getColumnNumber(startPosition);

    LOGGER.log(
      Level.WARNING,
      fileCounterStr +
        " Parsing " + node.getClass().getSimpleName() + " node at " +
        startLine + ":" + startColumn + " has been failed"
    );
  }

  public boolean isErrorDueParsing() {
    return errorDueParsing;
  }
}
