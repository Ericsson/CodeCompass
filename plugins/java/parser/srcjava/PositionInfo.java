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

  public PositionInfo(CompilationUnit cu, ASTNode node) {
    this.start = node.getStartPosition() + 1;
    this.end = node.getStartPosition() + node.getLength() + 1;
    this.startLine = cu.getLineNumber(start);
    this.startColumn = cu.getColumnNumber(start);

    int endLine = cu.getLineNumber(end);
    int endColumn = cu.getColumnNumber(end);

    // End position checking, and trying with a two less value.
    // This will wok in most cases, if the current element is a class
    // declaration and the newline character is missing from it's end.
    this.endLine =
      cu.getLineNumber(end) == -1 ?
        cu.getLineNumber(end - 2) :
        endLine;
    this.endColumn =
      cu.getColumnNumber(end) == -1 ?
        cu.getColumnNumber(end - 2) :
        endColumn;
  }

  public PositionInfo(CompilationUnit cu, ASTNode node, Javadoc javadoc) {
    this.start = javadoc.getStartPosition() + javadoc.getLength() + 1;
    this.end = node.getStartPosition() + node.getLength() + 1;
    this.startLine = cu.getLineNumber(start);
    this.startColumn = cu.getColumnNumber(start);

    int endLine = cu.getLineNumber(end);
    int endColumn = cu.getColumnNumber(end);

    // End position checking, and trying with a two less value.
    // This will wok in most cases, if the current element is a class
    // declaration and the newline character is missing from it's end.
    this.endLine =
      cu.getLineNumber(end) == -1 ?
        cu.getLineNumber(end - 2) :
        endLine;
    this.endColumn =
      cu.getColumnNumber(end) == -1 ?
        cu.getColumnNumber(end - 2) :
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