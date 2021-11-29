package parser.srcjava;

import cc.parser.java.*;
import org.apache.commons.io.FileUtils;
import org.apache.thrift.TException;
import org.eclipse.jdt.core.JavaCore;
import org.eclipse.jdt.core.dom.AST;
import org.eclipse.jdt.core.dom.ASTParser;
import org.eclipse.jdt.core.dom.CompilationUnit;

import javax.persistence.EntityManager;
import java.io.File;
import java.io.IOException;
import java.util.*;

import static model.EMFactory.createEntityManager;
import static parser.srcjava.Utils.*;

public class JavaParser implements JavaParserService.Iface {
  private static final String javaVersion;
  private static final ThreadLocal<EntityManager> em;
  private static final ASTParser parser;

  static {
    javaVersion = getJavaVersion();
    em = ThreadLocal.withInitial(() ->
      createEntityManager(
        System.getProperty("rawDbContext"), true
      )
    );
    parser = ASTParser.newParser(AST.JLS_Latest);
    parser.setKind(ASTParser.K_COMPILATION_UNIT);
  }

  @Override
  public synchronized ParseResult parseFile(
    CompileCommand compileCOmmand, long fileId,
    String fileCounterStr) throws TException
  {
    ArgParser argParser = new ArgParser(compileCOmmand);
    String filePath = argParser.getFilepath();

    try {
      File file = new File(filePath);
      String fileStr = FileUtils.readFileToString(file, "UTF-8");

      String[] classpathEntries =
        argParser.getClasspath().toArray(new String[0]);
      String[] sourcepathEntries =
        argParser.getSourcepath().toArray(new String[0]);
      String[] encodings = new String[sourcepathEntries.length];
      Arrays.fill(encodings, "UTF-8");

      parser.setResolveBindings(true);
      parser.setBindingsRecovery(true);
      parser.setCompilerOptions(getJavaCoreOptions());
      parser.setUnitName(argParser.getFilename());
      parser.setEnvironment(
        classpathEntries, sourcepathEntries,
        encodings, true
      );
      parser.setSource(fileStr.toCharArray());

      CompilationUnit cu = (CompilationUnit) parser.createAST(null);

      AstVisitor visitor = new AstVisitor(cu, em.get(), fileId);
      cu.accept(visitor);

      return getParseResult(cu, argParser, fileCounterStr);

    } catch (IOException e) {
      JavaBeforeParseException ex = new JavaBeforeParseException();
      ex.message = e.getMessage();
      throw ex;
    } catch (Exception e) {
      JavaParseException ex = new JavaParseException();
      ex.message = e.getMessage();
      throw ex;
    }
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
