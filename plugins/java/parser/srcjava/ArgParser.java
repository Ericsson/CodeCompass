package parser.srcjava;

import cc.parser.java.CompileCommand;
import org.json.simple.JSONObject;
import parser.srcjava.enums.ValidCommands;

import java.nio.file.Paths;
import java.util.*;

public class ArgParser {
  private static final String JREPathStr = System.getProperty("java.home");
  private final String directory;
  private final ArrayList<String> classpath;
  private final ArrayList<String> sourcepath;
  private final String filepath;
  private final String filename;

  public ArgParser(CompileCommand compileCommand) {
    this.directory = compileCommand.getDirectory();
    this.classpath = new ArrayList<>();
    this.sourcepath = new ArrayList<>();
    this.filepath = compileCommand.getFile();
    this.filename = Paths.get(this.filepath).getFileName().toString();

    parseCommand(compileCommand.getCommand());
  }

  private void parseCommand(String fullCommand) {
    this.classpath.add(JREPathStr);
    List<String> rawArgs = Arrays.asList(fullCommand.split(" "));
    Iterator<String> argIt = rawArgs.iterator();
    String command = argIt.next();

    if (Arrays.stream(ValidCommands.values())
        .map(ValidCommands::getName)
        .noneMatch(command::equals)) {
      throw new IllegalArgumentException("Unknown command.");
    }

    if (command.equals(ValidCommands.JAVAC.getName())) {
      parseJavacCommand(argIt);
    }
    /*
    UNDER CONSTRUCTION
    else if (command.equals(ValidCommands.MVN.getName())) {
    } etc...
    */
  }

  private void parseJavacCommand(
    Iterator<String> argIt)
    throws IllegalArgumentException
  {
    int filePathCounter = 0;

    while (argIt.hasNext()) {
      String nextArg = argIt.next();
      if (nextArg.startsWith("-")) {
        if (!argIt.hasNext()) {
          throw new IllegalArgumentException("Command has invalid syntax.");
        }
        String flagName = nextArg.substring(1);
        if (flagName.equals("cp") || flagName.equals("classpath")) {
          classpath.addAll(Arrays.asList(argIt.next().split(":")));
        } else if (flagName.equals("sourcepath")) {
          sourcepath.addAll(Arrays.asList(argIt.next().split(":")));
        }
      } else if (nextArg.equals(filepath)) {
        filePathCounter += 1;
      } else {
        throw new IllegalArgumentException("Command has invalid syntax");
      }
    }
    if (filePathCounter > 1) {
      throw new IllegalArgumentException("Command has invalid syntax");
    }
  }

  public static String getJREPathStr() {
    return JREPathStr;
  }

  public String getDirectory() {
    return directory;
  }

  public ArrayList<String> getClasspath() {
    return classpath;
  }

  public ArrayList<String> getSourcepath() {
    return sourcepath;
  }

  public String getFilepath() {
    return filepath;
  }

  public String getFilename() {
    return filename;
  }
}
