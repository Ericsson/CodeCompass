package parser.srcjava;

import cc.parser.java.*;
import com.strobel.decompiler.Decompiler;
import com.strobel.decompiler.DecompilerSettings;
import com.strobel.decompiler.PlainTextOutput;
import model.EMFactory;
import org.apache.commons.io.FileUtils;
import org.apache.thrift.TException;
import org.eclipse.jdt.core.dom.AST;
import org.eclipse.jdt.core.dom.ASTParser;
import org.eclipse.jdt.core.dom.CompilationUnit;

import javax.persistence.EntityManager;
import java.io.*;
import java.util.*;

import static parser.srcjava.Utils.*;

public class JavaParser implements JavaParserService.Iface {
  private final Hashtable<String, String> javaCoreOptions;
  private final ThreadLocal<EntityManager> em;
  private final ThreadLocal<ASTParser> parser;

  {
    javaCoreOptions = getJavaCoreOptions();
    EMFactory emf = new EMFactory(System.getProperty("rawDbContext"), true);
    em = ThreadLocal.withInitial(emf::createEntityManager);
    parser = ThreadLocal.withInitial(() -> ASTParser.newParser(AST.JLS_Latest));
    parser.get().setKind(ASTParser.K_COMPILATION_UNIT);
  }

  @Override
  public ParseResult parseFile(
    CompileCommand compileCOmmand, long fileId,
    String fileCounterStr) throws TException
  {
    ArgParser argParser = new ArgParser(compileCOmmand, fileCounterStr);
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

      parser.get().setResolveBindings(true);
      parser.get().setBindingsRecovery(true);
      parser.get().setCompilerOptions(javaCoreOptions);
      parser.get().setUnitName(argParser.getFilename());
      parser.get().setEnvironment(
        classpathEntries, sourcepathEntries,
        encodings, true
      );
      parser.get().setSource(fileStr.toCharArray());

      CompilationUnit cu = (CompilationUnit) parser.get().createAST(null);

      AstVisitor visitor = new AstVisitor(cu, em.get(), fileId, fileCounterStr);
      cu.accept(visitor);

      return getParseResult(
        cu, argParser, fileCounterStr, visitor.isErrorDueParsing());

    } catch (IOException e) {
      JavaBeforeParseException ex = new JavaBeforeParseException();
      ex.message = e.getMessage();
      throw ex;
    }
  }

  @Override
  public String decompileClass(String path) throws TException {
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

      settings.setIncludeLineNumbersInBytecode(false);
      settings.setIncludeErrorDiagnostics(false);
      settings.setForceExplicitImports(true);

      synchronized (this) {
        Decompiler.decompile(path, new PlainTextOutput(writer), settings);
      }
    }
    catch (Exception e) {
      ClassDecompileException ex = new ClassDecompileException();
      ex.message = e.getMessage();
      throw ex;
    }

    return javaFilePath;
  }
}
