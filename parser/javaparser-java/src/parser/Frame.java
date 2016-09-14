// $Id$
// Created by Aron Barath, 2013

package parser;

import java.util.HashMap;
import java.util.Map;

public class Frame
{
	Frame prev;
	String name;
	String qualifiedName;
	JavaType type;
	JavaFunction func;
	Map<String, JavaAstNode> vars;
	boolean mainFrame = false;
	int nextUniqueId = 1;

	public static final String GenerateUnique = new String();
	public static final JavaType InheritType = new JavaType();
	public static final JavaFunction InheritFunction = new JavaFunction();

	public static final char GeneratedFramePrefix = '_';

	/**
	 * Initialize a new frame.
	 * @param prev Previous frame.
	 * @param name Current context name (class name, mangled function name, Frame.GenerateUnique).
	 * @param type Type of the current frame. Used to store members and functions.
	 * @param func Current function. Used to store local variables.
	 */
	public Frame(Frame prev, String name, JavaType type, JavaFunction func, boolean over)
	{
		this.prev = prev;
		this.name = (name==GenerateUnique) ? uniqueName() : (name==null ? "" : name);
		this.type = (type==InheritType) ? ((prev!=null) ? prev.type : null) : type;
		this.func = (func==InheritFunction) ? ((prev!=null) ? prev.func : null) : func;

		if(!over && prev!=null && !prev.qualifiedName.isEmpty())
		{
			qualifiedName = prev.qualifiedName + "." + this.name;
		}
		else
		{
			qualifiedName = this.name;
		}

		if(prev==null)
		{
			vars = new HashMap<String, JavaAstNode>();
		}
		else
		{
			vars = new HashMap<String, JavaAstNode>(prev.vars);
		}
	}

	public Frame(Frame prev, String name, JavaType type, JavaFunction func)
	{
		this(prev, name, type, func, false);
	}

	public static Frame createFrame()
	{
		return createFrame("");
	}

	public static Frame createFrame(String pkg_name)
	{
		Frame f = new Frame(null, pkg_name, null, null, true);
		f.mainFrame = true;
		return f;
	}

	private String uniqueName()
	{
		return "" + GeneratedFramePrefix + (nextUniqueId++);
	}

	public JavaType getType()
	{
		return type;
	}

	public JavaFunction getFunction()
	{
		return func;
	}

	public String getName()
	{
		return name;
	}

	public String getQualifiedName()
	{
		return qualifiedName;
	}

	public Frame pop()
	{
		if(mainFrame)
		{
			return this;
		}
		
		return prev;
	}

	public void addVariable(String name, JavaAstNode var)
	{
		vars.put(name, var);
	}

	public JavaAstNode findVariable(String name)
	{
		return vars.get(name);
	}
}
