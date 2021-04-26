package parser.srcjava;

import org.json.simple.parser.ParseException;

import java.io.IOException;

public class Main {
  public static void main(String[] args) {
    try {
      Parser.parse(args[0]);
    } catch (IOException e) {
      e.printStackTrace();
    } catch (ParseException e) {
      e.printStackTrace();
    }
  }
}