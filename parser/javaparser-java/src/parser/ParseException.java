// $Id$
// Created by Aron Barath, 2014

package parser;

public class ParseException extends Exception
{
	private static final long serialVersionUID = -2059831230073491075L;

	public ParseException(String msg)
	{
		message = msg;
	}

	private String message;
	public String getMessage() { return message; }
}
