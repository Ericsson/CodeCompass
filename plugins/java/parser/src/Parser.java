import org.apache.commons.io.FileUtils;
import org.eclipse.jdt.core.JavaCore;
import org.eclipse.jdt.core.dom.*;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Map;
import java.util.stream.Stream;

public class Parser {
    private static final String[] CLASSPATH = { "C:\\Program Files\\Java\\jre1.8.0_261\\lib\\rt.jar" };

    public static void parse_project(String path) {

        try (Stream<Path> paths = Files.walk(Paths.get(path))) {
            paths.filter(p -> Files.isRegularFile(p) && p.toString().endsWith(".java")).forEach(p -> {
                try {
                    String str_path = p.toString();
                    System.out.println(p);
                    parse_file(str_path, new String[]{path});
                    System.out.println("----------------------------------------------------------------------------");
                } catch (IOException e) {
                    e.printStackTrace();
                }
            });

        } catch (IOException e){
            e.printStackTrace();
        }

    }

    private static void parse_file(String path, String[] sources) throws IOException {
        File file = new File(path);
        String str = FileUtils.readFileToString(file, "UTF-8");

        ASTParser parser = ASTParser.newParser(AST.JLS14);
        parser.setKind(ASTParser.K_COMPILATION_UNIT);

        Map<String, String> options = JavaCore.getOptions();
        parser.setCompilerOptions(options);

        parser.setEnvironment(CLASSPATH, sources, new String[] { "UTF-8" }, true);
        parser.setSource(str.toCharArray());

        CompilationUnit cu = (CompilationUnit) parser.createAST(null);

        AstVisitor visitor = new AstVisitor(cu);
        cu.accept(visitor);

        for (PositionInfo w : visitor.getWhileInfo()) {

            System.out.println("----------------------------------------------------------------------------");

        }

        /*
        System.out.println("----------------------------------------------------------------------------");
        System.out.println("Package information:");

        for (PackageDeclaration p : visitor.getPackageDeclarations()) {
            System.out.println("Package name: " + p.getName());
        }

        System.out.println("----------------------------------------------------------------------------");
        System.out.println("Single member annotations:");

        for (SingleMemberAnnotation a : visitor.getSingleMemberAnnotations()) {
            System.out.println("Annotation values: " + a.getValue());
        }

        System.out.println("----------------------------------------------------------------------------");
        System.out.println("Method declarations:");

        for (MethodDeclaration m : visitor.getMethodDeclarations()) {
            System.out.println("Method name: " + m.getName() + ", Return type: " + m.getReturnType2() + ", Parameters: " + m.parameters());
        }

        System.out.println("----------------------------------------------------------------------------");
        System.out.println("Method invocations:");

        for (MethodInvocation m : visitor.getMethodInvocations()) {
            System.out.println("Method name: " + m.getName() + ", Arguments: " + m.arguments() + ", Expression: " + m.getExpression());
        }

        System.out.println("----------------------------------------------------------------------------");
        System.out.println("String literals:");

        for (StringLiteral s : visitor.getStringLiterals()) {
            System.out.println("Literal value: " + s.getLiteralValue() + " Escaped value: " + s.getEscapedValue());
        }
        */
    }
}
