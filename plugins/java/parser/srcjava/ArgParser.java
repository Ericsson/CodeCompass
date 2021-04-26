package parser.srcjava;

import parser.srcjava.enums.ValidCommands;

import java.util.*;

public class ArgParser {
  private final String JREPathStr;

  public ArgParser() {
    JREPathStr = this.getJREPath();
  }

  public HashMap<String, ArrayList<String>> parse(
    String fullCommand, String filePathStr)
    throws IllegalArgumentException
  {
    HashMap<String, ArrayList<String>> argsMap = new HashMap<>();
    List<String> rawArgs = Arrays.asList(fullCommand.split(" "));
    Iterator<String> argIt = rawArgs.iterator();
    String command = argIt.next();

    if (Arrays.stream(ValidCommands.values())
        .map(ValidCommands::getName)
        .noneMatch(command::equals)) {
      throw new IllegalArgumentException("Unknown command.");
    }

    argsMap.put("classpath", new ArrayList<>(){{add(JREPathStr);}});
    argsMap.put("sourcepath", new ArrayList<>());

    if (command.equals(ValidCommands.JAVAC.getName())) {
      parseJavacCommand(argsMap, argIt, filePathStr);
    }
    /*
    UNDER CONSTRUCTION
    else if (command.equals(ValidCommands.MVN.getName())) {
    } etc...
    */

    return argsMap;
  }

  private String getJREPath() {
    return "";
  }

  private void parseJavacCommand(
    HashMap<String, ArrayList<String>> argsMap,
    Iterator<String> argIt, String filePathStr)
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
          Arrays.stream(argIt.next().split(":"))
            .forEach(cp -> argsMap.get("classpath").add(cp));
        } else if (flagName.equals("sourcepath")) {
          Arrays.stream(argIt.next().split(":"))
            .forEach(sp -> argsMap.get("sourcepath").add(sp));
        }
      } else if (nextArg.equals(filePathStr)) {
        filePathCounter += 1;
      } else {
        throw new IllegalArgumentException("Command has invalid syntax");
      }
    }
    if (filePathCounter > 1) {
      throw new IllegalArgumentException("Command has invalid syntax");
    }
  }
}
