package parser.srcjava;

import org.eclipse.jdt.core.dom.ASTNode;
import org.eclipse.jdt.core.dom.CompilationUnit;

public class PositionInfo {
  private final int start;
  private final int end;
  private final int startLine;
  private final int startColumn;
  private final int endLine;
  private final int endColumn;

  public PositionInfo(CompilationUnit cu, ASTNode node) {
    this.start = node.getStartPosition();
    this.end = node.getStartPosition() + node.getLength();
    this.startLine = cu.getLineNumber(start);
    this.startColumn = cu.getColumnNumber(start);
    this.endLine = cu.getColumnNumber(end);
    this.endColumn = cu.getColumnNumber(end);
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