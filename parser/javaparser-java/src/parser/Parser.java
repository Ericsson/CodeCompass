// $Id$
// Created by Aron Barath, 2013

package parser;

import static org.eclipse.persistence.config.PersistenceUnitProperties.CLASSLOADER;
import static org.eclipse.persistence.config.PersistenceUnitProperties.JDBC_DRIVER;
import static org.eclipse.persistence.config.PersistenceUnitProperties.JDBC_PASSWORD;
import static org.eclipse.persistence.config.PersistenceUnitProperties.JDBC_URL;
import static org.eclipse.persistence.config.PersistenceUnitProperties.JDBC_USER;
import static org.eclipse.persistence.config.PersistenceUnitProperties.TARGET_DATABASE;
import static org.eclipse.persistence.config.PersistenceUnitProperties.TARGET_SERVER;
import static org.eclipse.persistence.config.PersistenceUnitProperties.TRANSACTION_TYPE;

import java.io.FileReader;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;
import java.util.logging.Level;

import javax.persistence.EntityManager;
import javax.persistence.EntityManagerFactory;
import javax.persistence.Persistence;
import javax.persistence.TypedQuery;
import javax.persistence.spi.PersistenceUnitTransactionType;

import org.eclipse.jdt.core.JavaCore;
import org.eclipse.jdt.core.compiler.IProblem;
import org.eclipse.jdt.core.dom.*;
import org.eclipse.persistence.config.TargetServer;

import cc.parser.JavaParserArg;
import cc.parser.JavaParsingResult;


import javax.persistence.EntityTransaction;

public class Parser
{
	private static final String prefix_sqlite = "sqlite";
	private static final String prefix_pgsql_jdbc = "postgresql"; // JDBC pgsql prefix
	private static final String prefix_pgsql_cus = "pgsql"; // custom pgsql prefix

	private static final String driver_sqlite = "org.sqlite.JDBC";
	private static final String driver_pgsql = "org.postgresql.Driver";

	private static final Logger log = Logger.getLogger(Parser.class.getName());

	private static EntityManager createEntityManager(Object aClass, ConnStringParser csp)
	{
		// http://www.eclipse.org/forums/index.php?t=msg&goto=487011
		// http://wiki.eclipse.org/EclipseLink/Examples/JPA/OutsideContainer
		// http://www.eclipse.org/eclipselink/documentation/2.5/solutions/testingjpa.htm#BABEBCCJ

		HashMap<String, Object> props = new java.util.HashMap<String, Object>();

		props.put(CLASSLOADER, aClass.getClass().getClassLoader());

		props.put(TRANSACTION_TYPE, PersistenceUnitTransactionType.RESOURCE_LOCAL.name());

		String jdbcUrl = "jdbc:" + csp.getUrl();

		props.put(JDBC_DRIVER, csp.getDriver());
		props.put(JDBC_URL, jdbcUrl);
		props.put(JDBC_USER, csp.getUser());
		props.put(JDBC_PASSWORD, csp.getPassword());

		props.put(TARGET_SERVER, TargetServer.None);

		String databasePlatform;

		if(prefix_pgsql_jdbc.equals(csp.getPrefix()))
		{
			databasePlatform = "org.eclipse.persistence.platform.database.PostgreSQLPlatform";
		}
		else
		{
			databasePlatform = "parser.DatabasePlatform";
		}

		props.put(TARGET_DATABASE, databasePlatform);

		try
		{
			// Just for sure, store the unnecessary drivers in a list.

			String driver = csp.getDriver();
			List<java.sql.Driver> driversToDrop = new ArrayList<java.sql.Driver>();

			for(Enumeration<java.sql.Driver> e = java.sql.DriverManager.getDrivers(); e.hasMoreElements();)
			{
				java.sql.Driver drv = e.nextElement();

				if(!driver.equals(drv.getClass().getName()))
				{
					driversToDrop.add(drv);
				}
			}

			// Remove all drivers in 'driversToDrop'. 

			for(java.sql.Driver drv : driversToDrop)
			{
				log.log(Level.FINEST, "Deregister driver: " + drv.getClass().getName());
				java.sql.DriverManager.deregisterDriver(drv);
			}
		}
		catch(SQLException e)
		{
			e.printStackTrace();
		}

		EntityManagerFactory emf = Persistence.createEntityManagerFactory("ParserPU", props);

		return emf.createEntityManager();
	}
	
	private static long now()
	{
		return System.currentTimeMillis() / 1000L;
	}

	private static Project getProject(EntityManager em, long id)
	{
		Project project = em.find(Project.class, id);

		if(project==null)
		{
			project = new Project(id);
			em.persist(project);
		}

		return project;
	}

	private static void createNameOptionIfNotExist(EntityManager em, String value)
	{
		TypedQuery<Option> qs = em.createQuery("SELECT o FROM Option o WHERE o.key like \"name\"", Option.class);

		if(qs==null || qs.getResultList().isEmpty())
		{
			Option opt = new Option();
			opt.setKey("name");
			opt.setValue("");
			em.persist(opt);
		}
	}

	private static char[] loadContent(String filename)
	{
		try
		{
			FileReader reader = new FileReader(filename);
			StringBuilder builder = new StringBuilder();
			int initial_length = 256;
			char[] cbuf = new char[initial_length];
			int read_chars;

			while((read_chars=reader.read(cbuf))>0)
			{
				if(read_chars!=initial_length)
				{
					char[] copy = new char[read_chars];

					for(int i=0;i<read_chars;++i)
					{
						copy[i] = cbuf[i];
					}

					builder.append(copy);
				}
				else
				{
					builder.append(cbuf);
				}
			}

			reader.close();

			return builder.toString().toCharArray();
		}
		catch(Exception ex)
		{
			return null;
		}
	}

	private static int processProblems(int exitStatus, ProblemHandler ph)
	{
		if(0<ph.getProblems().size())
		{
			log.log(Level.FINEST, "There were " + ph.getProblems().size() + " problems.");

			for(ProblemHandler.Problem problem : ph.getProblems())
			{
				if(problem.isError())
				{
					exitStatus = 10;
				}

				log.log(Level.FINEST, "    " + problem.getProblemKind() + ": " + problem.getMessage());
			}
		}

		return exitStatus;
	}

	private static void applySourceLevel(String sourceLevel, Map<?, ?> options)
	{
		if("1.1".equals(sourceLevel))
		{
			JavaCore.setComplianceOptions(JavaCore.VERSION_1_1, options);
		}
		else
		if("1.2".equals(sourceLevel))
		{
			JavaCore.setComplianceOptions(JavaCore.VERSION_1_2, options);
		}
		else
		if("1.3".equals(sourceLevel))
		{
			JavaCore.setComplianceOptions(JavaCore.VERSION_1_3, options);
		}
		else
		if("1.4".equals(sourceLevel))
		{
			JavaCore.setComplianceOptions(JavaCore.VERSION_1_4, options);
		}
		else
		if("1.5".equals(sourceLevel) || "5".equals(sourceLevel))
		{
			JavaCore.setComplianceOptions(JavaCore.VERSION_1_5, options);
		}
		else
		if("1.6".equals(sourceLevel) || "6".equals(sourceLevel))
		{
			JavaCore.setComplianceOptions(JavaCore.VERSION_1_6, options);
		}
		else
		if("1.7".equals(sourceLevel) || "7".equals(sourceLevel))
		{
			JavaCore.setComplianceOptions(JavaCore.VERSION_1_7, options);
		}
		else
		{
			// defaulting to 1.7
			JavaCore.setComplianceOptions(JavaCore.VERSION_1_7, options);
		}
	}

	public static JavaParsingResult parse(JavaParserArg arg)
	{
		ArgParser ap = null;

		// FIXME: Should use the received arg instead of build a command line argument.
		List<String> args = new ArrayList<String>();
		args.add("--db");
		args.add(arg.database);
		args.add("--rt");
		args.add(arg.rtJar);
		args.add("--buildid");
		args.add(arg.buildId);
		args.add("-sourcepath");
		args.add(arg.sourcepath);
		args.addAll(arg.opts);

		try
		{
			ap = new ArgParser(args.toArray(new String[args.size()]));
		}
		catch(Exception ex)
		{
			System.out.println("Bad arguments.");
			return JavaParsingResult.Fail;
		}

		String database = ap.getDatabase();
		if(database.isEmpty())
		{
			// fail-safe sqlite database
			database = prefix_sqlite + ":database=javaparser.sqlite";
		}
		else
		if(database.endsWith(".sqlite") && !database.startsWith(prefix_sqlite))
		{
			// if database is not a proper sqlite connection string, then extend it
			database = prefix_sqlite + ":database=" + database;
		}

		if(!ap.isGood())
		{
			System.out.println("Bad arguments.");
			return JavaParsingResult.Fail;
		}

		log.log(Level.FINEST, "Orig.db.: " + database);

		ConnStringParser csp = new ConnStringParser("test");
		csp.addPrefix(prefix_sqlite, driver_sqlite);
		csp.addPrefix(prefix_pgsql_jdbc, driver_pgsql);
		csp.addAlias(prefix_pgsql_cus, prefix_pgsql_jdbc);
		if(!csp.parse(database))
		{
			return JavaParsingResult.Fail; 
		}

		String filename = ap.getFilename();
		System.out.println("Parsing " + filename);

		int exitStatus = 0;

		char[] source = loadContent(filename);
		if(source==null)
		{
			System.out.println("File open error: \"" + filename + "\".");
			return JavaParsingResult.Fail;
		}

		log.log(Level.FINEST, "Source:");
		log.log(Level.FINEST, "----------------------------------------------------");
		log.log(Level.FINEST, new String(source));
		log.log(Level.FINEST, "----------------------------------------------------");

		ASTParser parser = ASTParser.newParser(AST.JLS4);

		String driver = csp.getDriver();

		log.log(Level.FINEST, "Driver:   " + driver);
		log.log(Level.FINEST, "Database: " + database);
		
		EntityManager em = createEntityManager(parser, csp);
		em.getTransaction().begin();

		final long projectID = 1L; // this must be 1L
		Project project = getProject(em, projectID);

		String[] classpath = ap.getClassPathArray();
		String[] sourcepath = ap.getSourcePathArray();
		String[] encodings = new String[sourcepath.length];
		for(int i=0;i<encodings.length;++i)
		{
		  encodings[i] = "UTF-8";
		}

		log.log(Level.FINEST, "Classpath:");
		for(String s : classpath)
		{
			log.log(Level.FINEST, "    " + s);
		}
		log.log(Level.FINEST, "end of classpath");
		
		log.log(Level.FINEST, "Sourcepath:");
		for(String s : sourcepath)
		{
			log.log(Level.FINEST, "    " + s);
		}
		log.log(Level.FINEST, "end of sourcepath");

		parser.setSource(source);
		Map<?,?> options = JavaCore.getOptions();
		applySourceLevel(ap.getSourceLevel(), options);
		parser.setCompilerOptions(options);
		parser.setResolveBindings(true);
		parser.setBindingsRecovery(true);
		parser.setUnitName(filename);
		parser.setKind(ASTParser.K_COMPILATION_UNIT);
		parser.setEnvironment(classpath, sourcepath, encodings, false);

		try
		{
			CompilationUnit cu = (CompilationUnit)parser.createAST(null/*new org.eclipse.core.runtime.NullProgressMonitor()*/);

			if(cu!=null)
			{
				// if there is no "name" option, then create one
				createNameOptionIfNotExist(em, "");

				if(-1==filename.lastIndexOf('/'))
				{
					filename = ap.getWorkingDir() + "/" + filename;
				}

				int last_sep = filename.lastIndexOf('/');
				int path_sep = last_sep;

				while(path_sep>0 && filename.charAt(path_sep-1)=='/') { --path_sep; }

				ProblemHandler ph = new ProblemHandler();

				cu.accept(new AstVisitor(em, project, cu, ph, filename.substring(0, path_sep), filename.substring(last_sep+1), source, now(), ap.getBuildId(), ap.getCreateBuildAction()));

				exitStatus = processProblems(exitStatus, ph);
			}

			log.log(Level.FINEST, "~~~~~~~~~~~~~~~~~~~~~~~~~~");

			em.flush();
			em.getTransaction().commit();

			log.log(Level.FINEST, "Done.");

			if(exitStatus == 0)
				return JavaParsingResult.Success;
			else
				return JavaParsingResult.Fail;
		}
		catch(ParseException ex)
		{
			em.getTransaction().rollback();
			em.close();
			System.out.println("ERROR: " + ex.getMessage());
			return JavaParsingResult.Fail;
		}
		catch(Exception ex)
		{
			em.getTransaction().rollback();
			em.close();
			System.out.println("An exception has been caught.");
			ex.printStackTrace();
			return JavaParsingResult.Fail;
		}
	}
}