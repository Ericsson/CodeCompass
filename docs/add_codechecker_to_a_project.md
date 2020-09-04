We can analyze code by using the CodeCompass with `codechecker` subcommand.

# :exclamation: Prerequisites
* You must first parse your project first using CodeCompass parse command based on CodeCompass Parse Guide 
* Download and install the CodeChecker tool https://github.com/Ericsson/codechecker/tree/v5.10 (Version 5.10 works with CodeCompass)
# :star: Usage:
```
#set up the codechecker home directory variable to point to the root of your CodeChecker installation
export CODECHECKER_HOME=/path/to/Codechecker

CodeCompass codechecker [-h]  -w WORKDIR -n Name [-t THREAD]
                        [-e EXTRA]
                        [--dbhost DBHOST] [--dbport DBPORT]
                        [--dbuser DBUSER] [--dbpass DBPASS] 
```
**Note:** If your database is not located on localhost or you need to specify authentication to your database server then use *dbhost*, *dbport*, *dbuser* and *dbpass* parameters.

# :exclamation: Required parameters:
```
-w WORKDIR, --workdir WORKDIR         Path of workspace directory of the projects in which config files are stored.  
-n NAME, --name NAME                  Name of the project to check.
```

# :heavy_plus_sign: Additional parameters:
```
-h, --help                           Show this help message and exit 
-t THREAD, --thread THREAD           Number of threads to run CodeChecker on. The default value is 4.
-e EXTRA, --extra EXTRA              Extra arguments passed to the CodeChecker.
--dbhost DBHOST                      Address of the PgSQL server. The default value is  "localhost".
--dbhost DBPORT                      Port of the PgSQL server. The default value is 5432.
--dbhost DBUSER                      User name for the PgSQL server. The default value is "postgres".
--dbhost DBPASS                      Password file for the PgSQL server.
```

# :sparkles: Usage Example:
```bash
export CODECHECKER_HOME=/path/to/Codechecker
# Check helloworld project. We suppose that database is located on port "6250".
CodeCompass.py codechecker -w /home/username/workdir/ -n helloworld  --dbport 6250 --dbuser username

```
If all goes well, codecompass will have a CodeChecker slider on the left that shows the bugs in your code.
