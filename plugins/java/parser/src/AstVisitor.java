package parser.src;

import org.eclipse.jdt.core.dom.*;

public class AstVisitor extends ASTVisitor {

    private final CompilationUnit cu;

    public AstVisitor(CompilationUnit cu) {
        this.cu = cu;
    }

    @Override
    public boolean visit(PackageDeclaration node)
    {
        String packageName = node.getName().getFullyQualifiedName();
        
        return super.visit(node);
    }

    @Override
    public boolean visit(ImportDeclaration node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(TypeDeclaration node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(AnonymousClassDeclaration node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(EnumDeclaration node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(EnumConstantDeclaration node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(Block node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(MethodDeclaration node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(MethodInvocation node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(SuperMethodInvocation node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(SuperConstructorInvocation node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(ClassInstanceCreation node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(SingleVariableDeclaration node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(VariableDeclarationFragment node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(SimpleType node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(SimpleName node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(StringLiteral node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(AssertStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(BreakStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(ContinueStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(DoStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(EmptyStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(EnhancedForStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(ExpressionStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(ForStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(IfStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(LabeledStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(SwitchStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(SynchronizedStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(ThrowStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(TryStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(TypeDeclarationStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(VariableDeclarationStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }

    @Override
    public boolean visit(YieldStatement node)
    {
        // System.out.println(node);
        return super.visit(node);
    }
}