# Introduction
You can use the CodeCompass script in the CodeCompass/bin directory for parsing
and starting the webserver.

The parser can be started by using the `CodeCompass.py` with `parse` subcommand.

# :exclamation: Prerequisites
* Make sure that PostgresSQL is configured and started. This is a short guide on [[How To Set up PostgresSQL|Setup-PostgreSQL]]
* Create WORKDIR directory with write priviliges. All parselogs and the search index will be written into this directory. 

# :star: Usage

```
CodeCompass parse [-h]  -w WORKDIR -n NAME (-b BUILD | -l LOGFILE)
		     [-f] [-c] [-i] [--labels LABELS] [-e EXTRA]
		     [-t THREAD] 
		     [--dbhost DBHOST] [--dbport DBPORT]
		     [--dbuser DBUSER] [--dbpass DBPASS] 
```

**Note**: If your database is not located on localhost or you need to specify
authentication to your database server then use *dbhost*, *dbport*, *dbuser*
and *dbpass* parameters.

## :exclamation: Required parameters
```
-w WORKDIR, --workdir WORKDIR       Path of workspace directory of the projects in which config files are stored. 
-n NAME, --name NAME                Project name to identify the project.
-b BUILD, --build BUILD             This option is used to build the project and create build logs. 
                                    This flag has to be given the build command which builds the project.
-l LOGFILE, --logfile LOGFILE       Path of the parse log which is used by the parser as input.
``` 

## :heavy_plus_sign: Additional parameters
```
-h, --help                   Show this help message and exit 
-f, --force                  If -l is given and the log file already exists or -b is given and the project directory
                             already exists then the commands are not executed unless this flag is given.In case of
                             forcing the log file will be overwritten and project directory will be reused.
-c, --crash                  If this flag is given then parser stops when the parser crashes.
-i, --noindex                If this flag is given then the database indexes won't be added to the database.
--labels LABELS              Name paths with labels. Some CodeCompass parser components will parse only files under these
                             paths. Furthermore on the GUI there will be shortcuts on these paths. The string format 
                             has to be: label1=/path/one:label2=/path/two. 
                             If not given then an interactive interface will ask for the paths.
-e EXTRA, --extra EXTRA      Extra arguments passed to the CodeCompass webserver.
                             The parameter syntax should look like this: -e="extraparam1 extraparam2". 
                             Allowed values of the extra parameter: 
                                --skip-traverse                    Skip all traversal
                                --skip-parser                      Skip all language parser
                                --skip-pythonparser                Skip Python parser
                                --skip-similarity                  Skip similarity parser
                                --skip-version-control             Skip version control integration
                                --skip-docparser                   Don't parse doxygen documentation
-t THREAD, --thread THREAD   Number of threads to parse on. The default value is 4.
--dbhost DBHOST	             Address of the PgSQL server. The default value is  "localhost".
--dbhost DBPORT	             Port of the PgSQL server. The default value is 5432.
--dbhost DBUSER	             User name for the PgSQL server. The default value is "postgres".
--dbhost DBPASS	             Password file for the PgSQL server.
```

# :sparkles: Usage Example:
```bash
# Parse helloworld c++ project. We suppose that database is located on port 6250 on localhost:
cd /home/username/hello_world
CodeCompass parse -w /home/username/workdir -n helloworld -b "make" --dbuser username --dbport 6250 --labels "source=/home/username/hello_world"
```

This command does the following

* Creates search index for files in the */home/username/hello_world* directory. The search indexes stored into the  *{workdir}/helloworld/data* directory.
* Catches and parses the gcc,g++ calls invoked by make. The parsing output will be written into the *helloworld* database in pgsql.
* Logs are written into the *{workdir}/helloworld* directory.
* where *{workdir}* is */home/username/workdir*

# How to add CodeChecker (to show Clang Static Analyzer and Clang-Tidy warnings)

CodeCompass can show C/C++ bugs detected by the CodeChecker tool (https://github.com/Ericsson/codechecker).
There is a separate subcommand called *'codechecker'* to run CodeChecker on your parsing. Please follow the [CodeChecker Guide](add_codechecker_to_a_project.md).
# How to configure logger
When you execute CodeCompass parse, it runs a builds the project and logs gcc, g++ and javac command execution. In case your compiler is called other than these you can configure the logging process. In case you compile an ANT or Maven based project you may need to edit the compilation files.

You can configure the logging process as described the [Logging Configuration page](logging_configuration.md).

# How to check parse results

If you would like to check parse logs or files created for a specific workspace it is good to know the created directory structure:
```
{workdir}
    |_workspace1
        |_data #for search indexes, and GIT information
        |_dblog #log of postgres database operations
        |_parselog #log of the CodeCompass parsing
        |_build.json #log of the application build process
    |_workspace2
        |_...
    |_workspace3
        |_...
    |_webserver.log # log of the cdecompass webserver
    |_workspace.cfg # workspace configuration file describing the active workspaces. You do not need to manually maintain it.
```

Each parsing is stored in a unique workspace. When you start the CodeCompass webserver, it will automatically show all workspaces (in a dropdown list). 
