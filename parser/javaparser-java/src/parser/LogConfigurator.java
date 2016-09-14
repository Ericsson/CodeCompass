// $Id$
// Created by Aron Barath, 2015

package parser;

import java.io.ByteArrayInputStream;
import java.io.IOException;

import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.RollingFileAppender;

public class LogConfigurator
{
	static final Logger log = Logger.getLogger(LogConfigurator.class);

	public LogConfigurator()
	{
		final java.util.logging.LogManager logManager = 
			java.util.logging.LogManager.getLogManager();

		try
		{
			configureLog4j();

			String config = createConfig();
			ByteArrayInputStream cfgStream = new ByteArrayInputStream(config.getBytes());
			logManager.readConfiguration(cfgStream);
		}
		catch(IOException | SecurityException ex)
		{
			java.util.logging.Logger.getLogger(LogConfigurator.class.getName()).
			log(java.util.logging.Level.SEVERE, "Failed to configure custom log propeties!", ex);
		}
	}

	private static String createConfig()
	{
		String logLevel = System.getProperty("cc.javaparser.loglevel");
		if(logLevel == null)
		{
			logLevel = java.util.logging.Level.INFO.getName();
		}

		return
			"handlers = java.util.logging.ConsoleHandler\n" + 
			"cc.search.level = " + logLevel + "\n" +
			"java.util.logging.ConsoleHandler.level = ALL\n";
	}

	private static void configureLog4j()
	{
		Logger rootLogger = Logger.getRootLogger();

		String logLevel = System.getProperty("cc.javaparser.loglevel");
		Level log4jLevel = Level.WARN;
		switch(logLevel)
		{
			case "debug":
				log4jLevel = Level.DEBUG;
				break;

			case "info":
				log4jLevel = Level.INFO;
				break;

			case "warning":
				log4jLevel = Level.WARN;
				break;

			case "error":
				log4jLevel = Level.ERROR;
				break;

			case "critical":
				log4jLevel = Level.FATAL;
				break;

			case "status":
				log4jLevel = Level.INFO;
				break;

		}
		rootLogger.setLevel(log4jLevel);
		 
		//Define log pattern layout
		PatternLayout layout = new PatternLayout("%d{ISO8601} [%t] %-5p %c %x - %m%n");
		 
		//Add console appender to root logger
		rootLogger.addAppender(new ConsoleAppender(layout));
	}
}
