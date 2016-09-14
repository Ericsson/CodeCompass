// $Id$
// Created by Aron Barath, 2013

package parser;

import java.util.LinkedList;
import java.util.List;

import javax.persistence.TypedQuery;

public class Utils
{
	public static long fnvHash(String str)
	{
		long hash = 0xCBF29CE484222325L;

		for(int i=0,n=str.length();i<n;++i)
		{
			hash ^= (long)str.charAt(i);
			hash *= 1099511628211L;
		}

		// We have to avoid negative numbers.
		return hash & 0x7fffffffffffffffL;
	}

	public static String computeContentHash(char[] content)
	{
		try
		{
			java.security.MessageDigest sha1 = java.security.MessageDigest.getInstance("SHA-1");
			sha1.reset();
			sha1.update(content.toString().getBytes("utf8"));
			
			StringBuilder buf = new StringBuilder(64);

			for(byte b : sha1.digest())
			{
				buf.append(String.format("%02x", ((int)b) & 0xff));
			}

			return buf.toString();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return null;
	}

	public static long getNameHash(String str)
	{
		return fnvHash(str);
	}

	public static long getDocHtmlHash(String str)
	{
		return fnvHash(str);
	}

	public static List<String> splitPath(String path)
	{
		List<String> dirs = new LinkedList<String>();
		int idx = 0;
		int next_sep;

		while(path.length()>idx && path.charAt(idx)=='/') { ++idx; }

		while(path.length()>idx && (next_sep=path.indexOf('/', idx))>0)
		{
			dirs.add(path.substring(idx, next_sep));

			idx = next_sep;
			while(path.length()>idx && path.charAt(idx)=='/') { ++idx; }
		}

		if(path.length()>idx)
		{
			dirs.add(path.substring(idx));
		}

		return dirs;
	}

	public static JavaFunction findFunction(TypedQuery<JavaFunction> fs, String mangledName)
	{
		if(fs!=null)
		{
			for(JavaFunction f : fs.getResultList())
			{
				if(mangledName.equals(f.getMangledName()))
				{
					return f;
				}
			}
		}

		return null;
	}

	public static JavaType findType(TypedQuery<JavaType> ts, String qualName)
	{
		if(ts!=null)
		{
			for(JavaType t : ts.getResultList())
			{
				if(qualName.equals(t.getQualifiedName()))
				{
					return t;
				}
			}
		}

		return null;
	}

	public static File findFile(TypedQuery<File> fs, String path)
	{
		if(fs!=null)
		{
			for(File f : fs.getResultList())
			{
				if(path.equals(f.getPath()))
				{
					return f;
				}
			}
		}

		return null;
	}
}
