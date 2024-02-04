# CodeCompass usage

## Setup database (one time only)
CodeCompass uses relational database system for data storage. Currently *SQLite*
and *PostgreSQL* are supported. The first one is for trial purposes and for
smaller projects only, as SQLite can slow down considerably on large project.
PostgreSQL is recommended for any production operation on medium-size or larger
projects. The database backend used is decided when compiling CodeCompass, via
the `-DDATABASE` CMake flag (see chapter [Build CodeCompass](deps.md)).

### Using *SQLite*

The usage of SQLite is automatic, the embedded library will take care of
creating and using the database file.

### Using *PostgreSQL* from package manager

PostgreSQL can be installed from the package manager:

```bash
sudo apt install postgresql-<version>
# (e.g. postgresql-12 for Ubuntu 20.04
#   and postgresql-14 for Ubuntu 22.04)
```

This will set up an automatically starting local server on the default port
`5432`.

This server, by default, is only accessible for an automatically created system
user named `postgres`. However, CodeCompass's database layer only supports
password-based authentication. First, you have to create a user to access the
database:

```bash
sudo su postgres
psql
```

In the PostgreSQL command-line shell, type:

```sql
CREATE USER compass WITH CREATEDB LOGIN PASSWORD '<mypassword>';
```

You can exit this shell by typing `\q` and pressing the `ENTER` key. A user
with the given name and credentials is now created.

This was the most basic way to set up access to the database. In case of a live,
production, public server, certain other measures need to be taken to ensure
secure access. For full documentation, see:
- Read more about the [`CREATE USER`](https://www.postgresql.org/docs/12/sql-createuser.html)
  command.
- [PostgreSQL access configuration file](https://www.postgresql.org/docs/12/auth-pg-hba-conf.html)

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
- [Initialize PostgreSQL database](https://www.postgresql.org/docs/12/app-initdb.html)
  (`-E SQL_ASCII` flag is recommended!)
- [Start PostgreSQL database](https://www.postgresql.org/docs/12/app-postgres.html)

## 1. Generate compilation database
If you want to parse a C++ project, you have to create a [compilation database
file](http://clang.llvm.org/docs/JSONCompilationDatabase.html).

### Get compilation database from CMake
If the project uses CMake build system then you can create the compilation
database via CMake with the option `CMAKE_EXPORT_COMPILE_COMMANDS`:

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
CodeCompass_parser -w <workspace> -n <name> -i <input1> -i <input2> -d <connection_string>
```

- **Workspace**: This is a directory where some parse results and different
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

:exclamation: CodeCompass plugins are expected to individually handle what to
do when a parsing is stopped and restarted with an existing database, so the
parsing can continue from where it stopped.

### Usage example

In these examples, the parser is given **two** inputs. The
`compile_commands.json` file tells the C/C++ parser which source files to parse.
Giving the project's directory will allow the text search parser to index words
in your project.

Parse and store the results of the project in a PostgreSQL database:

```bash
CodeCompass_parser \
  -d "pgsql:host=localhost;port=5432;user=compass;password=mypassword;database=mydatabase" \
  -w ~/cc/workdir \
  -n myproject \
  -i ~/myproject/compile_commands.json \
  -i ~/myproject \
  -j 4
```

Or use SQLite (not recommended for large projects):

```bash
CodeCompass_parser \
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
Incremental parsing depends on the fact, that the build tool generates a **complete** compilation database, therefore the build commands for only the modified files are not sufficient.
In case of CMake, using the result of the `CMAKE_EXPORT_COMPILE_COMMANDS=ON` argument, the
compilation database will always contain all files.
Currently the C++ and metrics parsers support incremental parsing, while other parsers
just execute a forced reparse.

In case the analyzed software project was significantly changed (e.g. as a result of
restructuring the project), dropping the workspace database and performing a full, clean
parse can yield results faster. This can be achieved by passing the `--force` (or `-f`)
command line option which can be specified for `CodeCompass_parser`. Another solution is
to set the `--incremental-threshold` option, which configures an upper threshold of change
for incremental parsing (in the percentage of changed files). Above the threshold a full,
clean reparse is performed. The default value for this threshold is *10%*.

In order to review the changes detected by the incremental parser without performing any
action that would alter the workspace database or directory, the `--dry-run` command line
option can be specified for `CodeCompass_parser`.

## 3. Start the web server
You can start the CodeCompass webserver with `CodeCompass_webserver` binary in
the CodeCompass installation directory.

```bash
CodeCompass_webserver -w <workdir> -p <port>
```

- **Workspace**: This is a directory where the some parse results, and different
  configuration and log files are located. This should be the same as what was
  provided to the `CodeCompass_parser` binary in *Step 2*.
- **Port**: Port number of the web server to listen on.

For full documentation see `CodeCompass_webserver -h`.

### Enabling authentication

To enable this feature, an `authentication.json` file should be created under
the workspace directory (`--workspace` or `-w` flag given to the server).
At a bare minimum, to restrict access, an `"enabled": true` MUST be present
in the JSON.

For further details and examples, **see the guide on 
[Requiring authentication](authentication.md).**

### Enabling HTTPS (SSL/TLS) secure server

By default, CodeCompass starts a conventional, plain-text HTTP server on the
port specified.
In case a `certificate.pem` file exists under the `--workpace` directory, the
server *will* start in SSL mode.

The certificate file shall be in PEM format, which looks like shown below. If
the certificate you received from your Certificate Authority (or self-created)
isn't in PEM format, use an SSL tool like [OpenSSL](http://openssl.org) to
convert it.

Normally, the private and public key (the certificate) are created as separate
files. They **must** be concatenated to *one* `certificate.pem` file, to look
like the following.
[Further details on SSL](http://github.com/cesanta/mongoose/blob/5.4/docs/SSL.md#how-to-create-ssl-certificate-file)
is available from Mongoose, the library CodeCompass uses for HTTP server.

~~~{.pem}
-----BEGIN RSA PRIVATE KEY-----
MIIEogIBAAKCAQEAwONaLOP7EdegqjRuQKSDXzvHmFMZfBufjhELhNjo5KsL4ieH
hYN0Zii2yTb63jGxKY6gH1R/r9dL8kXaJmcZrfSa3AgywnteJWg=
-----END RSA PRIVATE KEY-----
-----BEGIN CERTIFICATE-----
MIIDBjCCAe4CCQCX05m0b053QzANBgkqhkiG9w0BAQQFADBFMQswCQYDVQQGEwJB
SEGI4JSxV56lYg==
-----END CERTIFICATE-----
~~~

> **Note:** Make sure your certificate file itself is not password-protected,
> as requiring the password to be entered will make the server unable to start
> on its own.

If intermediate certificates are used because your certificate isn't signed
by a Root CA (this is common), the certificate chain's elements (also in, or
converted to PEM format) should also be concatenated into the `certificate.pem`
file:

~~~{.pem}
-----BEGIN RSA PRIVATE KEY-----
Your certificate's private key
-----END RSA PRIVATE KEY-----
-----BEGIN CERTIFICATE-----
Your certificate (the public key)
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
The certificate of the CA that signed your certificate
-----END CERTIFICATE-----
~~~

### Enabling Google Analytics

To enable this feature, a `ga.txt` file should be created under
the workspace directory (`--workspace` or `-w` flag given to the server).
The `ga.txt` file shall consist of a single line, containing the 
Google Analytics *Tracking ID* (format `UA-XXXXXX-X`) or *Measurement ID* 
(format `G-XXXXXXX`).
When configured correctly, upon starting a `CodeCompass_webserver`, the 
*"Google Analytics monitoring enabled."* informational status message will 
be displayed.

**Note:** without explicitly creating the `ga.txt` file in the workspace 
directory, the Google Analytics integration is disabled and no related code 
is executed in the browser. **This is the default behaviour.**

### Usage example

```bash
# Start the server listening on port 6251.
CodeCompass_webserver \
  -w ~/cc/workdir \
  -p 6251
```

The server will be available in a browser on
[`http://localhost:6251`](http://localhost:6251).

### Logging

In both the parser and the webserver it is possible to write the logs to a given directory.
This feature can be enabled by passing the `--logtarget` command line option with the full
path to the directory to be used for storing the log files.
If this argument is not specified, the logs will be written to the terminal only.

### Language Server Protocol support

The CodeCompass_webserver is not a fully fledged LSP server on its own,
but it does support some standard and non-standard LSP requests for C and C++ projects.
The full list can be found here: [Supported LSP Requests](lsp.md)

To access this feature, requests must be sent to the following address:
`http://<domain>:<port>/<project_name>/CppLspService`

e.g.: [`http://localhost:6251/MyProject/CppLspService`](http://localhost:6251/MyProject/CppLspService)

The project name should match the name of the project used by the CodeCompass_parser.
