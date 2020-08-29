You can manage workspaces by using the `CodeCompass.py` with the `workspace` subcommand.

# :star: Usage:
```
CodeCompass.py workspace [-h]  -w WORKDIR (-l | -d NAME | -r FROM TO | -e WORKSPACE FILE | -i IMPORT)
		     [--dbhost DBHOST] [--dbport DBPORT]
		     [--dbuser DBUSER] [--dbpass DBPASS] 
```
**Note**: If your database is not located on localhost or you need to specify authentication to your database server then use `dbhost`, `dbport`, `dbuser` and `dbpass` parameters.

***!!!Warning!!!***  the *"rename"* and *"export"* operations currently don't rename and export the corresponding CodeChecker database if any.

# :exclamation: Required parameters:
```
-w WORKDIR, --workdir WORKDIR                Path of workspace directory of the projects in which config files are stored. 
-l, --list                                   List all workspaces.
-d NAME, --delete NAME                       Remove the whole workspace, including all config files and databases.
-r FROM TO, --rename FROM TO                 Rename workspace. The effect takes place in the database too.
-e WORKSPACE FILE, --export WORKSPACE FILE   Export the workspace into a .tar.gz file.
-i IMPORT, --import IMPORT                   Import a workspace from a project.tar.gz file.
``` 

# :heavy_plus_sign: Additional parameters:
```
-h, --help             Show this help message and exit 
--dbhost DBHOST        Address of the PgSQL server. The default value is  "localhost".
--dbhost DBPORT        Port of the PgSQL server. The default value is 5432.
--dbhost DBUSER        User name for the PgSQL server. The default value is "postgres".
--dbhost DBPASS        Password file for the PgSQL server.
```
# :sparkles: Usage Example:
```bash
# list all workspaces
CodeCompass.py workspace -w /home/username/workdir -l

# rename helloworld workspace to helloworld2 
CodeCompass.py workspace -w /home/username/workdir -r helloworld helloworld2

# export helloworld workspace
CodeCompass.py workspace -w /home/username/workdir -e helloworld exportedhellow.tar.gz

# import a saved workspace
CodeCompass.py workspace -w /home/username/workdir -i exportedhellow.tar.gz

# remove "helloword" workspace on port "6250" with username "username"
CodeCompass.py workspace -w /home/username/workdir -d helloworld --dbport 6250 --dbuser username
```
