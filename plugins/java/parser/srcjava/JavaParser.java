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
import cc.parser.java.CmdArgs;

import static parser.srcjava.Logger.LOGGER;
import static model.EMFactory.createEntityManager;

public class JavaParser implements JavaParserService.Iface {
  private static final String size;
  private static final String javaVersion;
  private static final EntityManager em;
  private static final ASTParser parser;
  private ArgParser argParser;

  static {
    size = System.getProperty("size");
    javaVersion = getJavaVersion();
    em = createEntityManager(System.getProperty("rawDbContext"));
    parser = ASTParser.newParser(AST.JLS_Latest);
    parser.setKind(ASTParser.K_COMPILATION_UNIT);
  }

  @Override
  public void parseFile(long fileId, int fileIndex) {
    String filePath = argParser.getFilepath();
    LOGGER.log(
      Level.INFO,
      "(" + fileIndex + "/" + size + ") " +
      "Parsing " + filePath);

    try {
      File file = new File(filePath);
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
      LOGGER.log(
        Level.SEVERE,
        "(" + fileIndex + "/" + size + ") " +
        "Parsing " + filePath + " failed");
    }
  }

  @Override
  public void setArgs(CompileCommand compileCommand) {
    argParser = new ArgParser(compileCommand);
  }

  @Override
  public CmdArgs getArgs() {
    CmdArgs cmdArgs = new CmdArgs();
    cmdArgs.directory = argParser.getDirectory();
    cmdArgs.classpath = argParser.getClasspath();
    cmdArgs.sourcepath = argParser.getSourcepath();
    cmdArgs.filepath = argParser.getFilepath();
    cmdArgs.filename = argParser.getFilename();

    return cmdArgs;
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
