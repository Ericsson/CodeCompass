// $Id$
// Created by Aron Barath, 2014

package parser;

import java.util.List;
import java.util.ArrayList;
import java.io.File;
import java.io.FilenameFilter;

public class ArgParser
{
	private class SkipArg
	{
		private String regex;
		private int params;

		public SkipArg(String regex, int params)
		{
			this.regex = regex;
			this.params = params;
		}

		public String getRegex() { return regex; }
		public int getParams() { return params; }
	}
	
	public ArgParser(String[] args)
	{
		if(args==null) { return; }

		List<SkipArg> skipArgs = new ArrayList<SkipArg>();
		skipArgs.add(new SkipArg("-target", 1));
		skipArgs.add(new SkipArg("-deprecation", 0));
		skipArgs.add(new SkipArg("-A.*", 0));
		skipArgs.add(new SkipArg("-D.*", 0));
		skipArgs.add(new SkipArg("-X.*", 0));
		skipArgs.add(new SkipArg("-J.*", 0));
		skipArgs.add(new SkipArg("-g.*", 0));
		skipArgs.add(new SkipArg("-proc:.*", 0));
		skipArgs.add(new SkipArg("-processor.*", 1));
		skipArgs.add(new SkipArg("-implicit:.*", 0));
		skipArgs.add(new SkipArg("-Werror", 0));
		skipArgs.add(new SkipArg("-verbose", 0));
		skipArgs.add(new SkipArg("-encoding", 1));
		skipArgs.add(new SkipArg("-nowarn", 0));
		skipArgs.add(new SkipArg("-bootclasspath", 1));
		skipArgs.add(new SkipArg("-extdirs", 1));
		skipArgs.add(new SkipArg("-endorseddirs", 1));
		skipArgs.add(new SkipArg("-s", 1));
		skipArgs.add(new SkipArg("-O", 0));

		for(int i=0;i<args.length;++i)
		{
			if('-'==args[i].charAt(0))
			{
				if("-classpath".equals(args[i]) || "-cp".equals(args[i]))
				{
					if((i+1)>=args.length) { notEnoughArg(args[i]); return; }

					addClassPath(args[i+1]);
					++i;
				}
				else
				if("--wd".equals(args[i]) || "-sourcepath".equals(args[i]))
				{
					if((i+1)>=args.length) { notEnoughArg(args[i]); return; }
					
					for(String s : args[i+1].split(":"))
					{
					  File path = new File(s);
					  if (path.exists())
					  {
					    sourcePath.add(s);
					  }
					}
					
					// FIXME: maybe it isn't correct
					if(!sourcePath.isEmpty())
					{
						workingDir = sourcePath.get(0);
					}
					
					++i;
				}
				else
				if("-d".equals(args[i]))
				{
					if((i+1)>=args.length) { notEnoughArg(args[i]); return; }

					outDir = args[i+1];
					++i;
				}
				else
				if("--rt".equals(args[i]))
				{
					if((i+1)>=args.length) { notEnoughArg(args[i]); return; }

					runtimeLib = args[i+1];
					++i;
				}
				else
				if("--db".equals(args[i]))
				{
					if((i+1)>=args.length) { notEnoughArg(args[i]); return; }

					database = args[i+1];
					++i;
				}
				else
				if("--buildid".equals(args[i]))
				{
					if((i+1)>=args.length) { notEnoughArg(args[i]); return; }

					buildId = args[i+1];
					++i;
				}
				else
				if("--createbuildaction".equals(args[i]))
				{
					createBuildAction = true;
				}
				else
				if("-source".equals(args[i]))
				{
					if((i+1)>=args.length) { notEnoughArg(args[i]); return; }

					sourceLevel = args[i+1];
					++i;
				}
				else
				{
					SkipArg skipArg = findSkipArg(skipArgs, args[i]);

					if(null!=skipArg)
					{
						int params = skipArg.getParams();
						if((i+params)>=args.length) { notEnoughArg(args[i]); return; }
						i += params;
					}
					else
					{
						error("Unknown switch: \"" + args[i] + "\".");
						return;
					}
				}
			}
			else
			{
				if(filename.length()>0)
				{
					error("Parser accepts only 1 source file.");
					error("Current source: \"" + filename + "\"");
					error("New source: \"" + args[i] + "\"");
					return;
				}

				filename = args[i];
			}
		}

		// requires: filename is not empty and full path
		if(filename.isEmpty())
		{
			error("Missing source file name.");
			return;
		}

		// requires: runtime library is not empty and full path
		if(runtimeLib.isEmpty() || runtimeLib.charAt(0)!='/')
		{
			error("Runtime library path must be empty or must start with '/'.");
			return;
		}

		// if the working directory is empty, guess it from the filename
		if(workingDir.isEmpty())
		{
			// in that case the filename must be full path
			if(filename.charAt(0)!='/')
			{
				error("If no working directory set, source file name must start with '/'.");
				return;
			}

			int lastIdx = filename.lastIndexOf('/');
			assert(lastIdx>=0); // see a few lines above

			workingDir = filename.substring(0, lastIdx);

			if(workingDir.isEmpty())
			{
				workingDir = "/";
			}
		}
		else
		{
			// if the working directory exists, extend the filename

			// requires: working directory is full path
			if(workingDir.charAt(0)!='/')
			{
				error("Working directory must be full path.");
				return;
			}

			filename = workingDir + "/" + filename;
		}

		if(filename.startsWith(workingDir))
		{
			filename = filename.substring(workingDir.length()+1);
		}

		if(outDir.isEmpty())
		{
			outDir = workingDir;
		}

		replaceWorkingDirectory();
		classPath.add(runtimeLib);
		extractClassPath();
		good = true;
	}

	private static SkipArg findSkipArg(List<SkipArg> skipArgs, String str)
	{
		for(SkipArg skipArg : skipArgs)
		{
			if(str.matches(skipArg.getRegex()))
			{
				return skipArg;
			}
		}

		return null;
	}

	private void notEnoughArg(String option)
	{
		error("Not enough arguments for '" + option + "' switch.");
	}

	private void error(String msg)
	{
		System.err.println(msg);
	}

	private void addClassPath(String cp)
	{
		for(String s : cp.split(":"))
		{
			classPath.add(s);
		}
	}

	private void replaceWorkingDirectory()
	{
		for(int i=0;i<classPath.size();++i)
		{
			String cp = classPath.get(i);
			char ch = cp.charAt(0);

			if('.'==ch && (1==cp.length() || '/'==cp.charAt(1)))
			{
				classPath.set(i, workingDir + cp.substring(1));
			}
			else
			if('/'!=ch)
			{
				classPath.set(i, workingDir + "/" + cp);
			}
		}
	}

	private void extractClassPath()
	{
		List<String> entries = new ArrayList<String>();

		for(String path : classPath)
		{
			boolean asterix = path.endsWith("*");

			if(asterix)
			{
				path = path.substring(0, path.length()-1);
			}
			
			File file = new File(path);

			if(null!=file)
			{
				if(file.isDirectory())
				{
					for(File sub : file.listFiles(new FilenameFilter()
						{
							public boolean accept(File dir, String name)
							{
								String lowercase = name.toLowerCase();
								return lowercase.endsWith(".jar");
							}
						}))
					{
						entries.add(sub.getAbsolutePath());
					}
				}
				
				if (file.exists())
				{
				  entries.add(file.getAbsolutePath());
				}
			}
		}

		classPath = entries;
	}

	private boolean good = false;
	public boolean isGood() { return good; }

	private String filename = "";
	public String getFilename() { return filename; }

	private String buildId = "";
	public String getBuildId() { return buildId; }

	private String workingDir = "";
	public String getWorkingDir() { return workingDir; }

	private String outDir = "";
	public String getOutDir() { return outDir; }

	private String runtimeLib = "";
	public String getRuntimeLib() { return runtimeLib; }

	private String database = "";
	public String getDatabase() { return database; }

	private boolean createBuildAction = false;
	public boolean getCreateBuildAction() { return createBuildAction; }

	private String sourceLevel = "";
	public String getSourceLevel() { return sourceLevel; }

	private List<String> classPath = new ArrayList<String>();
	public List<String> getClassPath() { return classPath; }
	public String[] getClassPathArray()
	{
		String[] arr = new String[classPath.size()];

		for(int i=0;i<arr.length;++i)
		{
			arr[i] = classPath.get(i);
		}

		return arr;
	}
	
	private List<String> sourcePath = new ArrayList<String>();
  public List<String> getSourcePath() { return sourcePath; }
  public String[] getSourcePathArray()
  {
    String[] arr = new String[sourcePath.size()];

    for(int i=0;i<arr.length;++i)
    {
      arr[i] = sourcePath.get(i);
    }

    return arr;
  }
}
