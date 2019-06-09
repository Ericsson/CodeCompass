# CodeCompass usage

## Setup database (one time only)
CodeCompass uses relational database system for data storage. Currently *SQLite*
and *PostgreSQL* are supported. The first one is for trial purposes and for
smaller projects only, as SQLite can slow down considerably on large project.
PostgreSQL is recommender for any production operation on medium-size or larger
projects. The database backend used is decided when compiling CodeCompass, via
the `-DDATABASE` CMake flag (see chapter [Build CodeCompass](deps.md)).

### Using *SQLite*

The usage of SQLite is automatic, the embedded library will take care of
creating and using the database file.

### Using *PostgreSQL* from package manager

PostgreSQL can be installed from the package manager, using
`sudo apt-get install postgresql-<version>` (e.g. `postgresql-9.5`). This will
set up an automatically starting local server on the default port `5432`.

This server, by default, is only accessible for an automatically created system
user named `postgres`. However, CodeCompass' database layer only supports
password-based authentication. First, you have to create a user to access the
database:

```bash
sudo su postgres
psql
```

In the PostgreSQL command-line shell, type:

```sql
CREATE USER compass WITH SUPERUSER LOGIN PASSWORD 'mypassword';
```

You can exit this shell by typing `\q` and pressing the `ENTER` key. A user
with the given name and credentials is now created.

This was the most basic way to set up access to the database. In case of a live,
production, public server, certain other measures need to be taken to ensure
secure access. For full documentation, see:
- Read more about the [`CREATE USER`](https://www.postgresql.org/docs/9.5/static/sql-createuser.html)
  command.
- [PostgreSQL access configuration file](https://www.postgresql.org/docs/9.5/static/auth-pg-hba-conf.html)

The databases inside PostgreSQL instance will be created automatically by the
CodeCompass parser, so the user needs rights for adding new database.

### Using self-compiled *PostgreSQL*
Alternatively, you may compile PostgreSQL by yourself. Using this method, you'll
need to manually initialise a server and start your own instance:

```bash
mkdir -p ~/cc/database
initdb -D ~/cc/database -E SQL_ASCII

# Start PostgreSQL server on port 6250.
postgres -D ~/cc/database -p 6250
```

A manually started PostgreSQL server is automatically configured to your user,
and your user only. No extra user creation or configuration needs to take place.

For full documentation see:
- [Initialize PostgreSQL database](https://www.postgresql.org/docs/9.5/static/app-initdb.html)
  (`-E SQL_ASCII` flag is recommended!)
- [Start PostgreSQL database](https://www.postgresql.org/docs/9.5/static/app-postgres.html)

## 1. Generate compilation database
If you want to parse a C++ project, you have to create a [compilation database
file](http://clang.llvm.org/docs/JSONCompilationDatabase.html).

### Get compilation database from CMake
If the project uses CMake build system then you can create the compilation
database by CMake with the option `CMAKE_EXPORT_COMPILE_COMMANDS`:

```bash
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1
```

This command creates the `compile_commands.json` file, this is the compilation
database.

### Create compilation database via build instrumentation
CodeCompass can also be used for generating compilation database. This is a more
general solution because it works for non-CMake projects. Suppose you have
`hello.cpp` in the current working directory.

```bash
CodeCompass_logger ~/myproject/compilation_commands.json "g++ hello.cpp"
```

The `CodeCompass_logger` is provided in the `bin` directory in the CodeCompass
installation. The first command line argument is the output file name and the
second argument is the build command which compiles your project. This can be a
simple compiler invocation or starting a build system.

## 2. Parse the project
For parsing a project with CodeCompass, the following command has to be emitted:

```bash
keepalive CodeCompass_parser -w <workspace> -n <name> -i <input1> -i <input2> -d <connection_string>
```

- **Workspace**: This is a directory where the some parse results, and different
  configuration and log files are located. **Please ensure that this directory
  is not located under any *input* folder, or under your project's source file
  tree.**
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
reason. This way a long parsing session restarts if an error happens.
CodeCompass plugins are expected to individually handle what to do when a
parsing is stopped and restarted with an existing database, so the parsing can
continue from where it stopped.

### Usage example

In these examples, the parser is given **two** inputs. The
`compile_commands.json` file tells the C/C++ parser which source files to parse.
Giving the project's directory will allow the text search parser to index words
in your project.

Parse and store the results of the project in a PostgreSQL database:

```bash
keepalive CodeCompass_parser \
  -d "pgsql:host=localhost;port=5432;user=compass;password=mypassword;database=mydatabase" \
  -w ~/cc/workdir \
  -n myproject \
  -i ~/myproject/compile_commands.json \
  -i ~/myproject \
  -j 4
```

Or use SQLite (not recommended for large projects):

```bash
keepalive CodeCompass_parser \
  -d "sqlite:database=~/cc/mydatabase.sqlite" \
  -w ~/cc/workdir \
  -n myproject \
  -i ~/myproject/compile_commands.json \
  -i ~/myproject \
  -j 4
```

### Incremental parsing

As an experimental feature CodeCompass supports incremental parsing, updating an
existing database and project workspace by detecting the added, deleted and modified files.  
Incremental parsing depends on that the build tooling generates a **complete** compilation database, 
therefore the build commands for only the modified files are not sufficient.
In case of CMake, using the result of the `CMAKE_EXPORT_COMPILE_COMMANDS=ON` argument, the 
compilation database will always contain all files.
Currently the C++ and metrics parsers support incremental parsing, while other parsers
just execute a forced reparse.

In order to review the changes detected by the incremental parser without performing any
action that would alter the workspace database or directory, the `--dry-run` command line 
option can be specified for `CodeCompass_parser`.

## 3. Start the web server
You can start the CodeCompass webserver with `CodeCompass_webserver` binary in
the CodeCompass installation directory.

```bash
keepalive CodeCompass_webserver -w <workdir> -p <port> -d <connection_string>
```

- **Workspace**: This is a directory where the some parse results, and different
  configuration and log files are located. This should be the same as what was
  provided to the `CodeCompass_parser` binary in *Step 2*.
- **Port**: Port number of the web server to listen on.
- **Database**: The plugins can use an SQL database as storage. By the
  connection string the user can give the location of a running database
  system. In the parsing session the database name could have been provided.
  This database name is written in a config file in the workspace directory, so
  it is useless to provide it at server start.

For full documentation see `CodeCompass_webserver -h`.

:exclamation: The `keepalive` is a script in the `bin` directory of CodeCompass
installation. This is used to keep CodeCompass alive if it crashes for some
reason.

### Usage example

```bash
# Start the server listening on port 6251.
keepalive CodeCompass_webserver \
  -w ~/cc/workdir \
  -p 6251 \
  -d "pgsql:host=localhost;port=5432;user=compass;password=mypassword"

# Or if SQLite database is used:
keepalive CodeCompass_webserver \
  -w ~/cc/workdir \
  -p 6251 \
  -d "sqlite:database=~/cc/mydatabase.sqlite"
```

The server will be available in a browser on
[`http://localhost:6251`](http://localhost:6251).
