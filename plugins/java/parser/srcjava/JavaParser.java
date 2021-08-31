package parser.srcjava;

import org.apache.commons.io.FileUtils;
import org.eclipse.jdt.core.JavaCore;
import org.eclipse.jdt.core.dom.AST;
import org.eclipse.jdt.core.dom.ASTParser;
import org.eclipse.jdt.core.dom.CompilationUnit;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

import javax.persistence.EntityManager;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.LogManager;
import java.util.logging.Logger;

import static model.EMFactory.createEntityManager;

public class JavaParser {
  private static Logger LOGGER = null;
  EntityManager em;
  private ASTParser parser;
  private final String javaVersion;

  static {
    InputStream stream = JavaParser.class.getClassLoader().
            getResourceAsStream("META-INF/logging.properties");

    try {
      LogManager.getLogManager().readConfiguration(stream);
      LOGGER= Logger.getLogger(JavaParser.class.getName());

    } catch (IOException e) {
      System.out.println(
              "Logger initialization for Java plugin has been failed."
      );
    }
  }

  public JavaParser() {
    this.javaVersion = getJavaVersion();
  }

  public void parse(String rawDbContext, String jsonPath)
    throws ParseException, IOException
  {
    boolean javaFound = false;
    JSONParser jsonParser = new JSONParser();
    JSONArray commands = (JSONArray) jsonParser.parse(new FileReader(jsonPath));

    for (Object c : commands) {
      if (c instanceof JSONObject) {
        String filePathStr = ((JSONObject) c).get("file").toString();
        if (Files.isRegularFile(Paths.get(filePathStr)) &&
            filePathStr.endsWith(".java")) {
          if (!javaFound) {
            javaFound = true;
            em = createEntityManager(rawDbContext);
            parser = ASTParser.newParser(AST.JLS_Latest);
            parser.setKind(ASTParser.K_COMPILATION_UNIT);
          }

          parseFile(filePathStr, new ArgParser((JSONObject) c));
        }
      } else {
        LOGGER.log(Level.SEVERE, "Command object has wrong syntax.");
      }
    }
  }

  private void parseFile(
    String filePathStr, ArgParser argParser)
    throws IOException
  {
    LOGGER.log(Level.INFO,"Parsing " + filePathStr);

    File file = new File(filePathStr);
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

    AstVisitor visitor = new AstVisitor(cu, em);
    cu.accept(visitor);
  }

  private Hashtable<String, String> getJavaCoreOptions() {
    Hashtable<String, String> options = JavaCore.getOptions();
    JavaCore.setComplianceOptions(javaVersion, options);

    return options;
  }

  private String getJavaVersion() {
    String javaCoreVersion = System.getProperty("java.version");
    int dot1 = javaCoreVersion.indexOf(".");

    if (javaCoreVersion.startsWith("1.")) {
      int dot2 = javaCoreVersion.indexOf(".", dot1 + 1);
      return javaCoreVersion.substring(0, dot2);
    }

    return javaCoreVersion.substring(0, dot1);
  }
}
