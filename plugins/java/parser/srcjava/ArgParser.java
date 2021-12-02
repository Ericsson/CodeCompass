package parser.srcjava;

import cc.parser.java.CompileCommand;
import parser.srcjava.enums.ValidCommands;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.logging.Level;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import static logger.Logger.LOGGER;

public class ArgParser {
  private static final String JREPathStr = System.getProperty("java.home");
  private final String directory;
  private final ArrayList<String> classpath;
  private final ArrayList<String> sourcepath;
  private final String filepath;
  private final String filename;
  private String bytecodePath;
  private List<String> bytecodesPaths;

  public ArgParser(CompileCommand compileCommand) {
    this.directory = compileCommand.getDirectory();
    this.classpath = new ArrayList<>();
    this.sourcepath = new ArrayList<>();
    this.filepath = compileCommand.getFile();
    this.filename = Paths.get(this.filepath).getFileName().toString();
    this.bytecodePath =
      this.filepath.substring(0, this.filepath.lastIndexOf('/'));

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
    boolean bytecodeDirFound = false;

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
        } else if (flagName.equals("d")) {
          if (!bytecodeDirFound) {
            bytecodeDirFound = true;
            bytecodePath = argIt.next();
          } else {
            throw new IllegalArgumentException("Command has invalid syntax");
          }
        } else {
          argIt.next();
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

    if (bytecodeDirFound && sourcepath.size() > 0) {
      for (String sp : sourcepath) {
        if (filepath.startsWith(sp)) {
          Path localBytecodePath =
            Paths.get(bytecodePath, filepath.substring(sp.length()));
          setBytecodesPaths(localBytecodePath);
        }
        break;
      }
    } else {
      setBytecodesPaths(Paths.get(bytecodePath));
    }
  }

  private void setBytecodesPaths(Path localBytecodePath) {
    try (Stream<Path> walk = Files.walk(localBytecodePath, 1)) {
      bytecodesPaths = walk
        .filter(this::bytecodeBelongsToFile)
        .map(Path::toString)
        .collect(Collectors.toList());
    } catch (IOException e) {
      LOGGER.log(
        Level.SEVERE, "Cannot find path into the generated .class files");
    }
  }

  private boolean bytecodeBelongsToFile(Path path) {
    String cleanFileName = filename.substring(0, filename.lastIndexOf('.'));
    String actualFilenameStr = path.getFileName().toString();

    return Files.isRegularFile(path) &&
      (actualFilenameStr.startsWith(cleanFileName) ||
        actualFilenameStr.startsWith(cleanFileName + '$')) &&
      actualFilenameStr.endsWith(".class");
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

  public String getBytecodePath() {
    return bytecodePath;
  }

  public List<String> getBytecodesPaths() {
    return bytecodesPaths;
  }

  public String[] getCommandLineArguments() {
    ArrayList<String> commandLineArgs = new ArrayList<>();

    if (!classpath.isEmpty()) {
      commandLineArgs.add("--classpath");
      commandLineArgs.addAll(classpath);
    }

    if (!sourcepath.isEmpty()) {
      commandLineArgs.add("--sourcepath");
      commandLineArgs.addAll(sourcepath);
    }

    commandLineArgs.add(filepath);

    return (String[]) commandLineArgs.toArray();
  }
}
