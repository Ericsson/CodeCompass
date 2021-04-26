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

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class Parser {
  public static void parse(String path) throws ParseException, IOException {
    JSONParser jsonParser = new JSONParser();
    JSONArray commands = (JSONArray) jsonParser.parse(new FileReader(path));

    ArgParser argParser = new ArgParser();

    for (Object c : commands) {
      if (c instanceof JSONObject) {
        String filePathStr = ((JSONObject) c).get("file").toString();
        if (Files.isRegularFile(Paths.get(filePathStr)) &&
            filePathStr.endsWith(".java")) {
          HashMap<String, ArrayList<String>> argsMap =
            argParser.parse(
              ((JSONObject) c).get("command").toString(),
              filePathStr
            );

          parse_file(filePathStr, argsMap);
        }
      } else {
        System.out.println("Command object has wrong syntax.");
      }
    }
  }

  private static void parse_file(
    String path, HashMap<String, ArrayList<String>> argsMap)
    throws IOException
  {
    File file = new File(path);
    String str = FileUtils.readFileToString(file, "UTF-8");

    ASTParser parser = ASTParser.newParser(AST.JLS_Latest);
    parser.setKind(ASTParser.K_COMPILATION_UNIT);

    Map<String, String> options = JavaCore.getOptions();
    parser.setCompilerOptions(options);
    parser.setEnvironment(
      argsMap.get("classpath").toArray(new String[0]), /*new String[]{"/asd"}, this version will not cause any errors, but I don't know why*/
      argsMap.get("sourcepath").toArray(new String[0]), /*new String[]{"/asd"},*/
      new String[]{"UTF-8"}, true);
    parser.setSource(str.toCharArray());

    CompilationUnit cu = (CompilationUnit) parser.createAST(null);

    AstVisitor visitor = new AstVisitor(cu);
    cu.accept(visitor);
  }
}
