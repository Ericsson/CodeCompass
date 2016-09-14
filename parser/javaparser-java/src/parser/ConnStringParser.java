// $Id$
// Created by Aron Barath, 2014

package parser;

import java.io.BufferedReader;
import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Logger;
import java.util.logging.Level;

public class ConnStringParser
{
	private static final Logger log = Logger.getLogger(Parser.class.getName());

	private String prefix;
	public String getPrefix() { return prefix; }

	private String driver;
	public String getDriver() { return driver; }

	private String database;
	public String getDatabase() { return database; }

	public String getUrl()
	{
		String url = prefix + ":";

		if(!host.isEmpty())
		{
			url += "//" + host;

			if(port>0)
			{
				url += ":" + port;
			}

			url += "/";
		}

		return url + database;
	}

	private String user;
	public String getUser() { return user; }

	private String initialUser;

	private String password;
	public String getPassword() { return password; }

	private String passwdfile;

	private int port;
	private String host;

	private Map<String, String> prefixAliasMap = new HashMap<String, String>();
	public void addAlias(String prefix, String alias)
	{
		prefixAliasMap.put(prefix, alias);
	}

	private Map<String, String> prefixDriverMap = new HashMap<String, String>();
	public void addPrefix(String prefix, String driver)
	{
		prefixDriverMap.put(prefix, driver);
	}

	public ConnStringParser(String user)
	{
		initialUser = user;
		reset();
	}

	public void reset()
	{
		driver = "";
		database = "";
		user = initialUser;
		password = "";
		passwdfile = "";
		port = -1;
		host = "";
	}

	private void printError(String msg)
	{
		log.log(Level.SEVERE, "ERROR: invalid connection string (" + msg + ")");
	}

	public boolean parse(String connstring)
	{
		// cut the prefix

		int colonPos = connstring.indexOf(':');
		if(colonPos<0)
		{
			// prefix is missing
			printError("prefix is missing");
			return false;
		}

		prefix = connstring.substring(0, colonPos);
		String alias = prefixAliasMap.get(prefix);
		if(null!=alias)
		{
			prefix = alias;
		}

		driver = prefixDriverMap.get(prefix);
		if(null==driver)
		{
			printError("unknown prefix: \"" + prefix + "\"");
			return false;
		}

		for(String keyvalue : connstring.substring(colonPos+1).split(";"))
		{
			// get key and value

			int eqPos = keyvalue.indexOf('=');

			if(eqPos<0)
			{
				// bad format
				printError("missing \"=\" sign");
				return false;
			}

			String key   = keyvalue.substring(0, eqPos);
			String value = keyvalue.substring(eqPos+1);

			// process key 

			if(!processKeyValue(key, value))
			{
				// something went wrong :(
				return false;
			}
		}

		// if no passwdfile fall-back to PGPASSFILE env var
		if(null==passwdfile || passwdfile.isEmpty())
		{
			String file = System.getenv().get("PGPASSFILE");
			if(null!=file)
			{
				passwdfile = file;
			}
		}

		// process passwdfile
		if(null!=passwdfile && !passwdfile.isEmpty())
		{
			return readPasswdfile();
		}

		return true;
	}

	private boolean readPasswdfile()
	{
		if(null==user || user.isEmpty())
		{
			printError("missing user name");
			return false;
		}

		try
		{
			BufferedReader reader = Files.newBufferedReader(FileSystems.getDefault().getPath(passwdfile), Charset.defaultCharset());
			String line = null;

			while(null!=(line=reader.readLine()))
			{
				if(!line.isEmpty() && '#'!=line.charAt(0))
				{
					// TODO: the line could contain escaped colons
					// for now: find the last colon
	
					String parts[] = line.split(":");
	
					if(5!=parts.length)
					{
						printError("invalid file passwdfile format");
						return false;
					}

					if(user.matches(parts[3]))
					{
						password = parts[4];
						return true;
					}
				}
			}
		}
		catch(IOException ex)
		{
			printError("passwdfile \"" + passwdfile + "\" is not found");
			return false;
		}

		printError("user \"" + user + "\" is not found in passwdfile");
		return false;
	}

	private boolean processKeyValue(String key, String value)
	{
		switch(key)
		{
		case "database":
			database = value;
			return true;

		case "user":
			user = value;
			return true;

		case "passwdfile":
			passwdfile = value;
			return true;

		case "password":
			password = value;
			return true;

		case "host":
			host = value;
			return true;

		case "port":
			try
			{
				port = Integer.parseInt(value);
				return true;
			}
			catch(NumberFormatException ex)
			{
				printError("Bad port: \"" + value + "\"");
				return false;
			}

		default:
			printError("unknown key: \"" + key + "\"");
			return false;
		}
	}
}
