package parser.src;

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
  private static final String[] CLASSPATH = {"C:\\Program Files\\Java\\jre1.8.0_261\\lib\\rt.jar"};

  public static void parse(String path) {

    try (Stream<Path> paths = Files.walk(Paths.get(path))) {
      paths.filter(p -> Files.isRegularFile(p) && p.toString().endsWith(".java")).forEach(p -> {
        try {
          String str_path = p.toString();
          System.out.println(p);
          parse_file(str_path, new String[]{path});
        } catch (IOException e) {
          e.printStackTrace();
        }
      });

    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  private static void parse_file(String path, String[] sources) throws IOException {
    File file = new File(path);
    String str = FileUtils.readFileToString(file, "UTF-8");

    ASTParser parser = ASTParser.newParser(AST.JLS_Latest);
    parser.setKind(ASTParser.K_COMPILATION_UNIT);

    Map<String, String> options = JavaCore.getOptions();
    parser.setCompilerOptions(options);

    parser.setEnvironment(CLASSPATH, sources, new String[]{"UTF-8"}, true);
    parser.setSource(str.toCharArray());

    CompilationUnit cu = (CompilationUnit) parser.createAST(null);

    AstVisitor visitor = new AstVisitor(cu);
    cu.accept(visitor);
  }
}
