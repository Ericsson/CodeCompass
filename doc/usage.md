# CodeCompass usage

## Setup database (one time only)
CodeCompass uses relational database system for data storage. Currently SQLite
and PostgreSQL are supported. The first one is for trial purposes and for
smaller projects, the latter one is for parsing large projects. You may choose
but make sure that CodeCompass is compiled with the given database system (see:
`-DDATABASE` CMake flag in chapter [Build CodeCompass](deps.md).

If you choose PostgreSQL then first the database server has to be initialized
and started. If PostgreSQL is installed by the Linux package manager then it is
done. Otherwise you can start your own instance:

```bash
mkdir -p ~/cc/database
initdb -D ~/cc/database -E SQL_ASCII

# Start PostgreSQL server on port 6250
postgres -D ~/cc/database -p 6250
```

For full documentation see:
- [Initialize PostgreSQL database](https://www.postgresql.org/docs/9.5/static/app-initdb.html)
  (`-E SQL_ASCII` flag is recommended!)
- [Start PostgreSQL database](https://www.postgresql.org/docs/9.5/static/app-postgres.html)

## 1. Generate compilation databse
If you want to parse a C++ project, you have to create a compilation database
file (http://clang.llvm.org/docs/JSONCompilationDatabase.html).

### Compilation database from CMake
If the project uses CMake build system then you can create the compilation
database by CMake with the option `CMAKE_EXPORT_COMPILE_COMMANDS`:

```bash
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1
```

This command creates the `compile_commands.json` file, this is the compilation
database.

### Compilation database from CodeCompass
CodeCompass can also be used for generating compilation database. This is a more
general solution because it is not only for CMake projects. Suppose you have
`hello.cpp` in the current working directory.

```bash
CodeCompass_logger ~/cc/compilation_commands.json "g++ main.cpp"
```

The `CodeCompass_logger` is provided in the `bin` directory in the CodeCompass
installation. The first command line argument is the output file name and the
second argument is the build command which compiles your project. This can be a
simple compiler invocation or starting a build system.

## 2. Parse a project
If you installed some third party dependecies manually for building CodeCompass
because of known issues then you have to make their `lib` directories seen:

```bash
export LD_LIBRARY_PATH=<thrift_install_dir>/lib:<clang_install_dir>/lib:<odb_install_dir>/lib:$LD_LIBRARY_PATH
```

For parsing a project with CodeCompass, the following command has to be emitted:

```bash
keepalive CodeCompass_parser -w <workspace> -n <name> -i <input1> -i <input2> -d <connection_string>
```

- **Workspace**: This is a directory where the parsed project and different
  config and log files are located.
- **Input**: Several inputs can be given with `-i` flags. An input can be a
  directory or a compilation database. The plugins iterate these inputs and
  decide if they can use it. For example the C++ parser will consume the given
  compilation databases and the text search parser will consume the files under
  the given directories. It is entirely up to the parser what it does with the
  input parameters.
- **Database**: The plugins can use an SQL database as storage. By the
  connection string the user can give the location of a running database
  system. If the database name is not given in the connection string then the
  project name will be used.

For full documentation see `CodeCompass_parser -h`.

:exclamation: The `keepalive` is a script in the `bin` directory of CodeCompass
installation. This is used to keep CodeCompass alive if it crashes for some
reason. This way a long parsing session restarts if an error happens. The
CodeCompass plugins should handle themselves what to do when a parsing is
stopped and restarted with an existing database, so the parsing can continue
from where it stopped. **Furthermore it sets some environment variables which
are necessary to start the `CodeCompass_parser` binary, so its usage is
mandatory!**

### Using example:

```bash
# Parse an example C++ project. We suppose that the database is located on port 6250 on localhost
keepalive CodeCompass_parser \
  -d "pgsql:host=localhost;database=mydatabase;port=6250" \
  -w ~/cc/workdir \
  -n myproject \
  -i ~/cc/compile_commands.json \
  -j 4
```

## 3. Start the webserver

You can start the CodeCompass weserver with `CodeCompass_webserver` binary in
the CodeCompass installation directory.

```bash
keepalive CodeCompass_webserver -w <workdir> -p <port> -d <connection_string>
```

- **Workspace**: This is a directory where the parsed projects and different
  config and log files are located. 
- **Port**: Port number of the webserver to listen on.
- **Database**: The plugins can use an SQL database as storage. By the
  connection string the user can give the location of a running database
  system. In the parsing session the database name could have been provided.
  This database name is written in a config file in the workspace directory, so
  it is useless to provide it at server start.

For full documentation see `CodeCompass_webserver -h`.

:exclamation: The `keepalive` is a script in the `bin` directory of CodeCompass
installation. This is used to keep CodeCompass alive if it crashes for some
reason. **Furthermore it sets some environment variables which are necessary to
start the `CodeCompass_parser` binary, so its usage is mandatory!**

### Using example:

```bash
# Start the server listening on port 6251. We suppose that database is located on port 6250.
keepalive CodeCompass_webserver \
  -w ~/cc/workdir \
  -p 6251 \
  -d "pgsql:host=localhost;database=mydatabase;port=6250"

# Open up a browser and access http://localhost:6251
```
