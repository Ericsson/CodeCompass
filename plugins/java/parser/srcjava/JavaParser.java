package parser.srcjava;

import org.apache.commons.io.FileUtils;
import org.eclipse.jdt.core.JavaCore;
import org.eclipse.jdt.core.dom.AST;
import org.eclipse.jdt.core.dom.ASTParser;
import org.eclipse.jdt.core.dom.CompilationUnit;

import javax.persistence.EntityManager;
import java.io.File;
import java.io.IOException;
import java.util.*;
import java.util.logging.Level;

import cc.parser.java.JavaParserService;
import cc.parser.java.CompileCommand;

import static parser.srcjava.Logger.LOGGER;
import static model.EMFactory.createEntityManager;

public class JavaParser implements JavaParserService.Iface {
  private static String javaVersion;
  private static EntityManager em;
  private static ASTParser parser;

  public JavaParser(String rawDbContext) {
    javaVersion = getJavaVersion();
    em = createEntityManager(rawDbContext);
    parser = ASTParser.newParser(AST.JLS_Latest);
    parser.setKind(ASTParser.K_COMPILATION_UNIT);
  }

  @Override
  public int parseFile(CompileCommand compileCommand, long fileId) {
    String fileName = compileCommand.getFile();
    ArgParser argParser = new ArgParser(compileCommand);
    LOGGER.log(Level.INFO,"Parsing " + fileName);

    try {
      File file = new File(fileName);
      String fileStr = FileUtils.readFileToString(file, "UTF-8");

      parser.setResolveBindings(true);
      parser.setBindingsRecovery(true);
      parser.setCompilerOptions(getJavaCoreOptions());
      parser.setUnitName(argParser.getFilename());
      parser.setEnvironment(
        argParser.getClasspath().toArray(new String[0]),
        argParser.getSourcepath().toArray(new String[0]),
        new String[]{"UTF-8"}, false
      );
      parser.setSource(fileStr.toCharArray());

      CompilationUnit cu = (CompilationUnit) parser.createAST(null);

      AstVisitor visitor = new AstVisitor(cu, em, fileId);
      cu.accept(visitor);
    } catch (IOException e) {
      LOGGER.log(Level.SEVERE, "Parsing " + fileName + " failed");
      return 1;
    }

    return 0;
  }

  private static String getJavaVersion() {
    String javaCoreVersion = System.getProperty("java.version");
    int dot1 = javaCoreVersion.indexOf(".");

    if (javaCoreVersion.startsWith("1.")) {
      int dot2 = javaCoreVersion.indexOf(".", dot1 + 1);
      return javaCoreVersion.substring(0, dot2);
    }

    return javaCoreVersion.substring(0, dot1);
  }

  private Hashtable<String, String> getJavaCoreOptions() {
    Hashtable<String, String> options = JavaCore.getOptions();
    JavaCore.setComplianceOptions(javaVersion, options);

    return options;
  }
}
