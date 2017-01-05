// $Id$
// Created by Aron Barath, 2014

package parser;

public class NameExtractor
{
	String qualifier;
	String name;
	String arguments;

	private NameExtractor(String mangledName, boolean isType)
	{
		int idx;

		String qualName;
		if(!isType && (idx=mangledName.lastIndexOf('('))>0)
		{
			qualName = mangledName.substring(0, idx);
			arguments = mangledName.substring(idx);
		}
		else
		{
			qualName = mangledName;
			arguments = "";
		}

		if((idx=findLastDot(qualName))>0)
		{
			qualifier = qualName.substring(0, idx);
			name = qualName.substring(idx + 1);
		}
		else
		{
			qualifier = "";
			name = qualName;
		}

		if(isType)
		{
			assert(arguments.isEmpty());

			if((idx=name.indexOf('<'))>0)
			{
				arguments = name.substring(idx);
				name = name.substring(0, idx);
			}
		}
	}

	private static int findLastDot(String str)
	{
		return findPrevChar(str, '.', str.length()-1);
	}

	private static int findPrevChar(String str, char ch, int idx)
	{
		for(;idx>=0;--idx)
		{
			char at = str.charAt(idx);

			if(at==ch)
			{
				return idx;
			}
			else
			if(at=='>')
			{
				idx = findPrevChar(str, '<', idx-1);
			}
		}

		return -1;
	}

	public static NameExtractor fromFunctionName(String mangledName)
	{
		return new NameExtractor(mangledName, false);
	}

	public static NameExtractor fromTypeName(String mangledName)
	{
		return new NameExtractor(mangledName, true);
	}

	private static String removeGenericParams(String s)
	{
		String result = "";

		int len = s.length();
		int skipLevel = 0;

		for(int i=0;i<len;++i)
		{
			if(skipLevel==0 && s.charAt(i)!='<')
			{
				result += s.charAt(i);
			}
			else
			if(s.charAt(i)!='>')
			{
				assert(skipLevel>0);
				--skipLevel;
			}
			else
			{
				++skipLevel;
			}
		}

		return result;
	}

	public NameExtractor removeGenericParams()
	{
		name = removeGenericParams(name);
		qualifier = removeGenericParams(qualifier);
		arguments = removeGenericParams(arguments);

		return this;
	}

	public String getName()
	{
		return name;
	}

	public String getQualifier()
	{
		return qualifier;
	}

	public String getQualName()
	{
		if(qualifier.isEmpty())
		{
			return name;
		}
		else
		{
			return qualifier + "." + name;
		}
	}

	public String getNameWithArgs()
	{
		return name + arguments;
	}

	public String getFullName()
	{
		return getQualName() + arguments;
	}
}
