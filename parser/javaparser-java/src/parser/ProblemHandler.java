// $Id$
// Created by Aron Barath, 2014

package parser;

import java.util.ArrayList;
import java.util.List;

public class ProblemHandler
{
	public enum ProblemKind
	{
		JavacWarning,
		JavacError,
		ParseError
	}

	public class Problem
	{
		private Problem(ProblemKind kind, String message)
		{
			this.kind = kind;
			this.message = message;
		}

		private ProblemKind kind;
		private String message;

		public ProblemKind getProblemKind() { return kind; }
		public String getMessage() { return message; }

		public boolean isError() { return ProblemKind.JavacError==kind || ProblemKind.ParseError==kind; }
	}

	private List<Problem> problems = new ArrayList<Problem>();
	public List<Problem> getProblems() { return problems; }

	public ProblemHandler()
	{
		// ok
	}

	public void add(ProblemKind kind, String message)
	{
		problems.add(new Problem(kind, message));
	}

	public void addJavacWarning(String message)
	{
		add(ProblemKind.JavacWarning, message);
	}

	public void addJavacError(String message)
	{
		add(ProblemKind.JavacError, message);
	}

	public void addJavacProblem(boolean isError, String message)
	{
		add(isError ? ProblemKind.JavacError : ProblemKind.JavacWarning, message);
	}

	public void addParseError(String message)
	{
		add(ProblemKind.ParseError, message);
	}
}
