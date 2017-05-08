# CodeCompass command line examples

## 0. Prerequisites
### Setup PostgreSQL (one time only)
Before the first use, you have to setup PostgreSQL (included in the binary package). PostgreSQL stores it's data files in a data directory, so before you start the PostgreSQL server you have to create and init this data directory.

```bash
source /home/<username>/cc/CodeCompass-deps/env.sh

mkdir -p /home/<username>/cc/database
initdb -D /home/<username>/cc/database -E "SQL_ASCII"
```

Alternatively you can use a PostgreSQL 9.3+ server installed to your system, but in this case you will need a database user account (with a password) and you should set the passwdfile parameter in your connection string (see below).


### Starting PostgreSQL server

You can run the database server with the following command:
```bash
cd /home/<username>/cc/database
postgres -D . -p <port>
```

### Create database
```bash
createdb <db_name> -p <port>
```

### Prepare an example project for parse
```bash
# Create a project folder
mkdir -p /home/<username>/cc/projects
cd /home/<username>/cc/projects

# Clone an example cmake based project
git clone https://github.com/leethomason/tinyxml2 <project_name>
mkdir -p <project_name>/build
cd <project_name>/build
```

### Generate compilation databse
If you want to parse a C++ project, you have to create a compilation database file. You can create this by cmake with the option `CMAKE_EXPORT_COMPILE_COMMANDS` (http://clang.llvm.org/docs/JSONCompilationDatabase.html). 
```bash
# Generate compilation database with cmake
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1
```
*Note:* it will be automatized later.

## 1. Parse a project
For parsing a project with CodeCompass, the following command has to be emitted:

```bash
CodeCompass_parser -w <workspace> -n <name> -i <input1> -i <input2> -d <connection_string>
```
- **Workspace**: This is a directory where the parsed project and different config and log files are located.
- **Input**: Several inputs can be given with -i flags. An input can be a directory or a compilation database. The plugins iterate these inputs and decide if it can use it. For example the C++ parser will consume the given compilation databases and the text search parser will consume the files under the given directories. It is entirely up to the parser what it does with the input parameters.
- **Database**: The plugins can use an SQL database as storage. By the connection string the user can give the location of a running database system. 

### Using example:
```bash
# Parse the prepared project
./home/<username>/cc/CodeCompass-install/bin/CodeCompass_parser -d "pgsql:database=<db_name>;port=<port>" -w /home/<username>/cc/workdir -i /home/<username>/cc/projects/<project_name>/build/compile_commands.json -j<num_of_threads>
```

## 2. Start the webserver

### Start the webserver
You can start the CodeCompass weserver with `bin/CodeCompass_webserver` binary in the CodeCompass installation directory.

```bash
CodeCompass_webserver -w <workdir> -p <port> -d <connection_string>
```
- **Workspace**: This is a directory where the parsed projects and different config and log files are located. 
- **Port**: Port number of the webserver to listen on.
- **Database**: The plugins can use an SQL database as storage. By the connection string the user can give the location of a running database system. 

### Using example:
```bash
./CodeCompass-install/bin/CodeCompass_webserver -w /home/<username>/cc/workdir/ -p <port> -d "pgsql:host=localhost;database=<db_name>;user=<username>;port=<port>"
```
