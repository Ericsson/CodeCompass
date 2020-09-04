You can start the CodeCompass webserver by using the `CodeCompass` with the `server` subcommand.

# :star: Usage:
```
CodeCompass.py server [-h]  -w WORKDIR [-p PORT] [-e EXTRA] [-s]
		      [-c CHECKERPORT]
		      [--dbhost DBHOST] [--dbport DBPORT]
		      [--dbuser DBUSER] [--dbpass DBPASS] 
```
**Note**: If your database is not located on localhost or you need to specify authentication to your database server then use *dbhost*, *dbport*, *dbuser* and *dbpass* parameters.

# :exclamation: Required parameters:
```
-w WORKDIR, --workdir WORKDIR              Path of workspace directory of the projects in which config files are stored.  
```
# :heavy_plus_sign: Additional parameters:
```
-h, --help                                  Show this help message and exit 
-p PORT, --port PORT                        Port number on which webserver listens. The default value is 8080.
-e EXTRA, --extra EXTRA                     Extra arguments passed to the CodeCompass webserver.
-s, --stop                                  If this flag is given, then the CodeCompass webserver belonging to the
                                            given workspace stops.
-c CHECKERPORT, --checkerport CHECKERPORT   The port number on which CodeChecker server will listen. The default value
                                            is 8081. For using CodeChecker the CODECHECKER_HOME environment variable
                                            has to contain the path of the CodeChecker' directory.
--dbhost DBHOST                             Address of the PgSQL server. The default value is  "localhost".
--dbhost DBPORT                             Port of the PgSQL server. The default value is 5432.
--dbhost DBUSER                             User name for the PgSQL server. The default value is "postgres".
--dbhost DBPASS                             Password file for the PgSQL server.
```

# :sparkles: Usage Example:
```bash
# Start the server on port "6251". We suppose that database is located on port "6250".
CodeCompass.py server -w /home/username/workdir/ -p 6251 --dbport 6250 --dbuser username
```
