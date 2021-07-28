package parser.srcjava;

import org.json.simple.parser.ParseException;

import java.io.IOException;

public class Main {
  public static void main(String[] args) {
    try {
      JavaParser javaParser = new JavaParser();
      javaParser.parse(args[0], args[1]);
    } catch (IOException | ParseException e) {
      e.printStackTrace();
    }
  }
}