How to use
==========

Configure environment
---------------------
Source the `cc-env.sh` file.

Now the important binaries are available on the `PATH` environment variable:  
`CodeCompass_parser`, `CodeCompass_webserver`, `CodeCompass_logger`, `ccdb-tool`.


Start a database
----------------
The tarball contains a CodeCompass instance linked against PostgreSQL, therefore two options are available.
  - Use the PostgreSQL database server on your machine.
  - The extracted tarball contains a PostgreSQL installation in the `runtime-deps/postgresql-install/bin` folder.
    Use the `initdb` and `postgres` binaries to create and start a new PostgreSQL database cluster.
