package parser.srcjava;

import cc.parser.java.*;
import com.strobel.decompiler.Decompiler;
import com.strobel.decompiler.DecompilerSettings;
import com.strobel.decompiler.PlainTextOutput;
import org.apache.commons.io.FileUtils;
import org.apache.thrift.TException;
import org.eclipse.jdt.core.dom.AST;
import org.eclipse.jdt.core.dom.ASTParser;
import org.eclipse.jdt.core.dom.CompilationUnit;

import javax.persistence.EntityManager;
import java.io.*;
import java.util.*;

import static model.EMFactory.createEntityManager;
import static parser.srcjava.Utils.*;

public class JavaParser implements JavaParserService.Iface {
  private static final Hashtable<String, String> javaCoreOptions;
  private static final ThreadLocal<EntityManager> em;
  private static final ASTParser parser;

  static {
    javaCoreOptions = getJavaCoreOptions();
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
      Arrays.fill(encodings, argParser.getEncoding());

      parser.setResolveBindings(true);
      parser.setBindingsRecovery(true);
      parser.setCompilerOptions(javaCoreOptions);
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
      e.printStackTrace();
      JavaBeforeParseException ex = new JavaBeforeParseException();
      ex.message = e.getMessage();
      throw ex;
    } catch (Exception e) {
      e.printStackTrace();
      JavaParseException ex = new JavaParseException();
      ex.message = e.getMessage();
      throw ex;
    }
  }

  @Override
  public synchronized String decompileClass(String path) throws TException {
    String javaFilePath =
      String.join(
        ".", path.substring(0, path.lastIndexOf('.')),
        "java"
      );

    try (
      FileOutputStream stream =
        new FileOutputStream(javaFilePath);
      OutputStreamWriter writer = new OutputStreamWriter(stream))
    {
      DecompilerSettings settings = new DecompilerSettings();

      settings.setForceExplicitImports(true);

      Decompiler.decompile(path, new PlainTextOutput(writer), settings);
    }
    catch (Exception e) {
      ClassDecompileException ex = new ClassDecompileException();
      ex.message = e.getMessage();
      throw ex;
    }

    return javaFilePath;
  }
}
