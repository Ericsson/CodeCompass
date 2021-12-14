package parser.srcjava;

import org.eclipse.jdt.core.dom.ASTNode;
import org.eclipse.jdt.core.dom.CompilationUnit;
import org.eclipse.jdt.core.dom.Javadoc;

public class PositionInfo {
  private final int start;
  private final int end;
  private final int startLine;
  private final int startColumn;
  private final int endLine;
  private final int endColumn;

  public PositionInfo(CompilationUnit cu, int startPosition, int endPosition) {
    this.start = startPosition + 1;
    this.end = endPosition + 2;
    this.startLine = cu.getLineNumber(start);
    this.startColumn = cu.getColumnNumber(start);

    int endLine = cu.getLineNumber(end);
    int endColumn = cu.getColumnNumber(end);

    /*
      End position checking, and trying with a two less value.
      This will work in most cases, if the current element is a class
      declaration and the newline character is missing from it's end.
    */
    this.endLine =
      cu.getLineNumber(this.end) == -1 ?
        cu.getLineNumber(this.end - 2) :
        endLine;
    this.endColumn =
      cu.getColumnNumber(this.end) == -1 ?
        cu.getColumnNumber(this.end - 2) :
        endColumn;
  }

  public PositionInfo(CompilationUnit cu, ASTNode node) {
    int startPosition = node.getStartPosition();
    this.start = startPosition + 1;
    this.end = startPosition + node.getLength() + 1;
    this.startLine = cu.getLineNumber(start);
    this.startColumn = cu.getColumnNumber(start);

    int endLine = cu.getLineNumber(end);
    int endColumn = cu.getColumnNumber(end);

    /*
      End position checking, and trying with a two less value.
      This will work in most cases, if the current element is a class
      declaration and the newline character is missing from it's end.
    */
    this.endLine =
      cu.getLineNumber(this.end) == -1 ?
        cu.getLineNumber(this.end - 2) :
        endLine;
    this.endColumn =
      cu.getColumnNumber(this.end) == -1 ?
        cu.getColumnNumber(this.end - 2) :
        endColumn;
  }

  public PositionInfo(CompilationUnit cu, ASTNode node, Javadoc javadoc) {
    this.start = javadoc.getStartPosition() + javadoc.getLength() + 2;
    this.end = node.getStartPosition() + node.getLength() + 1;
    this.startLine = cu.getLineNumber(start);
    this.startColumn = cu.getColumnNumber(start);

    int endLine = cu.getLineNumber(end);
    int endColumn = cu.getColumnNumber(end);

    /*
      End position checking, and trying with a two less value.
      This will work in most cases, if the current element is a class
      declaration and the newline character is missing from it's end.
    */
    this.endLine =
      cu.getLineNumber(this.end) == -1 ?
        cu.getLineNumber(this.end - 2) :
        endLine;
    this.endColumn =
      cu.getColumnNumber(this.end) == -1 ?
        cu.getColumnNumber(this.end - 2) :
        endColumn;
  }

  public int getStart() {
    return start;
  }

  public int getEnd() {
    return end;
  }

  public int getStartLine() {
    return startLine;
  }

  public int getStartColumn() {
    return startColumn;
  }

  public int getEndLine() {
    return endLine;
  }

  public int getEndColumn() {
    return endColumn;
  }
}