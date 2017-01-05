// $Id$
// Created by Aron Barath, 2013

package parser;

import org.eclipse.jdt.core.dom.IMethodBinding;
import org.eclipse.jdt.core.dom.ITypeBinding;
import org.eclipse.jdt.core.dom.MethodDeclaration;
import org.eclipse.jdt.core.dom.SingleVariableDeclaration;

public class NameMangler
{
	private StringBuilder nameBuilder = new StringBuilder();
	private StringBuilder paramBuilder = new StringBuilder();
	private boolean needParamSeparator = false;

	public NameMangler(Frame frame, MethodDeclaration method)
	{
		String name = frame.getQualifiedName();

		if(name.isEmpty())
		{
			nameBuilder.append(method.getName().getIdentifier());
		}
		else
		{
			nameBuilder.append(name);
			nameBuilder.append('.');
			nameBuilder.append(method.getName().getIdentifier());
		}

		beginParams();

		for(Object o : method.parameters())
		{
			if(o instanceof SingleVariableDeclaration)
			{
				SingleVariableDeclaration par = (SingleVariableDeclaration)o;

				addParam(par.getType().resolveBinding().getQualifiedName());
			}
		}

		endParams();
	}

	public NameMangler(Frame frame, IMethodBinding methbind)
	{
		assert(methbind!=null);

		nameBuilder.append(methbind.getDeclaringClass().getQualifiedName());
		nameBuilder.append('.');
		nameBuilder.append(methbind.getName());

		beginParams();

		for(ITypeBinding typeBind : methbind.getParameterTypes())
		{
			addParam(typeBind.getQualifiedName());
		}

		endParams();
	}

	public static String getMangledName(Frame frame, MethodDeclaration method)
	{
		return (new NameMangler(frame, method)).getMangledName();
	}

	public static String getMangledName(Frame frame, IMethodBinding methbind)
	{
		return (new NameMangler(frame, methbind)).getMangledName();
	}

	public static String getParameterTypes(Frame frame, IMethodBinding methbind)
	{
		return (new NameMangler(frame, methbind)).getParameters();
	}

	public String getMangledName()
	{
		return nameBuilder.toString() + paramBuilder.toString();
	}

	public String getName()
	{
		return nameBuilder.toString();
	}

	public String getParameters()
	{
		return paramBuilder.toString();
	}

	private void beginParams()
	{
		paramBuilder.append('(');
	}

	private void endParams()
	{
		paramBuilder.append(')');
	}

	private void addParam(String typeName)
	{
		if(needParamSeparator)
		{
			paramBuilder.append(',');
		}
		else
		{
			needParamSeparator = true;
		}

		paramBuilder.append(typeName);
	}
}
