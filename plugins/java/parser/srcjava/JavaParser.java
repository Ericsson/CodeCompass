package parser.srcjava;

import org.apache.commons.io.FileUtils;
import org.eclipse.jdt.core.dom.AST;
import org.eclipse.jdt.core.dom.ASTParser;
import org.eclipse.jdt.core.dom.CompilationUnit;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;

import static model.EMFactory.createEntityManager;

public class JavaParser {
  // EntityManager em;
  private ASTParser parser;

  public JavaParser() {}

  public void parse(String path) throws ParseException, IOException {
    boolean javaFound = false;
    JSONParser jsonParser = new JSONParser();
    JSONArray commands = (JSONArray) jsonParser.parse(new FileReader(path));

    ArgParser argParser = new ArgParser();

    for (Object c : commands) {
      if (c instanceof JSONObject) {
        String filePathStr = ((JSONObject) c).get("file").toString();
        if (Files.isRegularFile(Paths.get(filePathStr)) &&
            filePathStr.endsWith(".java")) {
          if (!javaFound) {
            javaFound = true;
            /* em = */
            createEntityManager();
            parser = ASTParser.newParser(AST.JLS_Latest);
            parser.setKind(ASTParser.K_COMPILATION_UNIT);
          }
          HashMap<String, ArrayList<String>> argsMap =
            argParser.parse(
              ((JSONObject) c).get("command").toString(), filePathStr
            );

          parseFile(filePathStr, argsMap);
        }
      } else {
        System.out.println("Command object has wrong syntax.");
      }
    }
  }

  private void parseFile(
    String path, HashMap<String, ArrayList<String>> argsMap)
    throws IOException
  {
    File file = new File(path);
    String str = FileUtils.readFileToString(file, "UTF-8");

    /*
    parser.setEnvironment(
      argsMap.get("classpath").toArray(new String[0]),
      argsMap.get("sourcepath").toArray(new String[0]),
      new String[]{"UTF-8"}, false
    );
    */
    parser.setSource(str.toCharArray());

    CompilationUnit cu = (CompilationUnit) parser.createAST(null);
    AstVisitor visitor = new AstVisitor(cu, null);
    cu.accept(visitor);
  }
}
