# :star: Setup PostgreSQL (one time only)
Before the first use, you have to setup PostgreSQL (included in the binary package). PostgreSQL stores its data files in a data directory, so before you start the PostgreSQL server you have to create and init this data directory. I'll call the data directory to pgsql_data.

Do the following steps:
```bash
mkdir -p /path/to/pgsql_data
initdb -D /path/to/pgsql_data -E "SQL_ASCII"
```

Alternatively you can use a PostgreSQL 9.3+ server installed to your system, but in this case you will need a database user account (with a password) and you should set the passwdfile parameter in your connection string (see below).

# :sparkles: Starting PostgreSQL server
You can run the server with the following command:
```bash
postgres -D /path/to/pgsql_data >pgsql_log 2>&1
```

*You must have a running PostgreSQL server if you want to run the parser or the web server.*

