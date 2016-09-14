// $Id$
// Created by Aron Barath, 2014

package parser;

import org.eclipse.jdt.core.compiler.IProblem;
import org.eclipse.jdt.core.dom.ASTNode;
import org.eclipse.jdt.core.dom.CompilationUnit;

public class PositionInfo
{
	public PositionInfo(CompilationUnit cu, IProblem problem)
	{
		this(cu, problem.getSourceStart(), problem.getSourceEnd());
	}

	public PositionInfo(CompilationUnit cu, ASTNode node)
	{
		this(cu, node.getStartPosition(), node.getStartPosition()+node.getLength());
	}

	public PositionInfo(CompilationUnit cu, ASTNode node, int offset)
	{
		this(cu, node.getStartPosition()+offset, node.getStartPosition()+node.getLength());
	}

	public PositionInfo(CompilationUnit cu, int start, int end)
	{
		this.start = start;
		this.end = end;
		startLine = cu.getLineNumber(start);
		startColumn = cu.getColumnNumber(start) + 1;
		endLine = cu.getLineNumber(end);
		endColumn = cu.getColumnNumber(end) + 1;
	}

	private int start;
	public int getStart() { return start; }

	private int end;
	public int getEnd() { return end; }

	private int startLine;
	public int getStartLine() { return startLine; }

	private int startColumn;
	public int getStartColumn() { return startColumn; }

	private int endLine;
	public int getEndLine() { return endLine; }

	private int endColumn;
	public int getEndColumn() { return endColumn; }
}
