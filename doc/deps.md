# Build Environment
We build CodeCompass in a Linux environment. Currently, Ubuntu Long-Term
Support releases are the main targets: Ubuntu 18.04 LTS and Ubuntu 20.04 LTS.

We also provide a Docker image that can be used as developer environment to
CodeCompass. See its usage [in a seperate document](/docker/README.md).

# Dependencies
The following third-party tools are needed for building CodeCompass. These can
be installed from the official repository of the given Linux distribution.

- **`git`**: For fetching and managing CodeCompass source code.
- **`cmake`** and **`make`**: For building CodeCompass.
- **`g++`**: For compiling CodeCompass. A version which supports C++14 features
  is required. (Alternatively, you can compile with Clang.)
- **`gcc-X`, `gcc-X-plugin-dev`**: For building ODB.
- **`libboost-all-dev`**: Boost can be used during the development.
- **`llvm-10-dev`**, **`clang-10`**, **`libclang-10-dev`**: C++ parser uses
  LLVM/Clang for parsing the source code.
- **`odb`**, **`libodb-dev`**: For persistence ODB can be used which is an
  Object Relation Mapping (ORM) system.
- **`libsqlite3-dev`**, **`libodb-sqlite-dev`**: SQLite library and the
  corresponding ODB development library, in case SQLite database system is
  used.
- **`postgresql-server-dev-<version>`**, **`libodb-pgsql-dev`**: PostgreSQL
  server and the corresponding ODB development library in case PostgreSQL
  database system is used.
- **`default-jdk`**: For search parsing CodeCompass uses an indexer written in
  Java.
- **`thrift-compiler`**, **`libthrift-dev`**: Thrift is used as the Web RPC API.
- **`libssl-dev`** / **`libssl1.0-dev`**: OpenSSL libs are required by Thrift,
  and NodeJS.
- **`libgraphviz-dev`**: GraphViz is used for generating diagram
  visualizations.
- **`libmagic-dev`**: For detecting file types.
- **`libgit2-dev`**: For compiling Git plugin in CodeCompass.
- **`npm`**: For handling JavaScript dependencies for CodeCompass web GUI.
- **`ctags`**: For search parsing.
- **`doxygen`**: For documentation generation.
- **`libgtest-dev`**: For testing CodeCompass.
  ***See [Known issues](#known-issues)!***
- **`libldap2-dev`**: For LDAP authentication.
- **`python3.8`**: For python parsing.

## Quick guide

The following command installs the packages except for those which have some
known issues.

#### Ubuntu 18.04 ("Bionic Beaver") LTS

```bash
sudo apt install git cmake make g++ gcc-7-plugin-dev libboost-all-dev \
  llvm-10-dev clang-10 libclang-10-dev \
  default-jdk libssl1.0-dev libgraphviz-dev libmagic-dev libgit2-dev ctags doxygen \
  libldap2-dev libgtest-dev npm python3.8
```

#### Ubuntu 20.04 ("Focal Fossa") LTS

```bash
sudo apt install git cmake make g++ libboost-all-dev \
  llvm-10-dev clang-10 libclang-10-dev \
  odb libodb-dev thrift-compiler libthrift-dev \
  default-jdk libssl-dev libgraphviz-dev libmagic-dev libgit2-dev ctags doxygen \
  libldap2-dev libgtest-dev npm
```

#### Database engine support

Depending on the desired database engines to be supported, the following
packages should be installed:

##### Ubuntu 18.04 ("Bionic Beaver")

The database connector library must be compiled manually for this release,
however, the database programs themselves should be installed from the
package manager.

```bash
# For SQLite database systems:
sudo apt install libsqlite3-dev

# For PostgreSQL database systems:
sudo apt install postgresql-server-dev-<version>
```

##### Ubuntu 20.04 ("Focal Fossa") LTS

```bash
# For SQLite database systems:
sudo apt-get install libodb-sqlite-dev libsqlite3-dev

# For PostgreSQL database systems:
sudo apt-get install libodb-pgsql-dev postgresql-server-dev-<version>
```


## Known issues
Some third-party tools are present in the distribution's package manager in a
way that they are either incompatible with each other or not available as a
package, thus can't be used to create your CodeCompass installation.

:warning: Building and installing from source code to the system is a dangerous
operation - outside package managers, the wrong binaries can end up being used
by other processes which could, in extreme cases, make the system very hard or
impossible to recover. **Please do NOT add a `sudo` in front of any `make` or
other commands below, unless *explicitly* specified!**

### ODB (for Ubuntu 18.04)
ODB is an Object Relational Mapping tool, that is required by CodeCompass.
For Ubuntu 18.04, the official release of ODB conflicts with the official
compiler (GNU G++ 7) of the distribution. A newer version of ODB must be
compiled manually.

The ODB installation uses the build2 build system. (Build2 is not needed for
CodeCompass so you may delete it right after the installation of ODB.)

```bash
wget https://download.build2.org/0.14.0/build2-install-0.14.0.sh
sh build2-install-0.14.0.sh --yes --trust yes "<build2_install_dir>"
```

Now, utilizing the *Build2* toolchain, we can build the *ODB* compiler and
libraries. In the script below, we assume that ODB is built in the
`<odb_build_dir>` directory and installed in the `<odb_install_dir>` directory.

```bash
export PATH="<build2_install_dir>/bin:$PATH"

# Configuring the build
cd <odb_build_dir>
bpkg create --quiet --jobs $(nproc) cc \
  config.cxx=g++ \
  config.cc.coptions=-O3 \
  config.bin.rpath=<odb_install_dir>/lib \
  config.install.root=<odb_install_dir>

# Getting the source
bpkg add https://pkg.cppget.org/1/beta --trust-yes
bpkg fetch --trust-yes

# Building odb
bpkg build odb --yes
bpkg build libodb --yes
bpkg build libodb-sqlite --yes
bpkg build libodb-pgsql --yes
bpkg install --all --recursive
```

Please take into consideration that the ODB installation can take up a long
time (depending on the machine one is using).

> **Note:** now you may delete the *Build2* toolchain installed in the
> `<build2_install_dir>` folder, if you do not need any longer.

### Thrift (for Ubuntu 18.04)
CodeCompass needs [Thrift](https://thrift.apache.org/) which provides Remote
Procedure Call (RPC) between the server and the client. A suitable version of
Thrift is, unfortunately, not part of the official Ubuntu repositories for
these versions, so you should download and build from source.

Thrift can generate stubs for many programming languages. The configure
script looks at the development environment and if it finds the environment
for a given language then it'll use it. For example in the previous step npm
was installed which requires NodeJS. If NodeJS can be found on your machine
then the corresponding stub will also compile. If you don't need it then you
can turn it off: `./configure --without-nodejs`.

In certain cases, installation may fail if development libraries for
languages are not installed on the target machine. E.g. if Python is
installed but the Python development headers are not, Thrift will unable to
install. Python, PHP and such other Thrift builds are NOT required by
CodeCompass, and can significantly increase compile time so it is advised to
avoid using them if it's not necessary.

```bash
# Download and uncompress Thrift:
wget "http://www.apache.org/dyn/mirrors/mirrors.cgi?action=download&filename=thrift/0.13.0/thrift-0.13.0.tar.gz" \
  -O thrift-0.13.0.tar.gz
tar -xvf ./thrift-0.13.0.tar.gz
cd thrift-0.13.0

./configure --prefix=<thrift_install_dir> --silent --without-python \
  --enable-libtool-lock --enable-tutorial=no --enable-tests=no      \
  --with-libevent --with-zlib --without-nodejs --without-lua        \
  --without-ruby --without-csharp --without-erlang --without-perl   \
  --without-php --without-php_extension --without-dart              \
  --without-haskell --without-go --without-rs --without-haxe        \
  --without-dotnetcore --without-d --without-qt4 --without-qt5      \
  --without-java

make install -j $(nproc)
```

### GTest/Googletest
The `libgtest-dev` package contains only the source files of GTest, but the
binaries are missing. You have to compile GTest manually.

```bash
mkdir gtest
cp -R /usr/src/googletest/* ./gtest

cd gtest
mkdir build
cd build

cmake .. -DCMAKE_INSTALL_PREFIX=<gtest_install_dir>
make install -j $(nproc)
```

# Build CodeCompass
The previously self-compiled and installed dependencies are not automatically
seen by CMake. Please set this environment before executing the build.

```bash
export GTEST_ROOT=<gtest_install_dir>

# If using Ubuntu 18.04:
export CMAKE_PREFIX_PATH=<thrift_install_dir>:$CMAKE_PREFIX_PATH
export CMAKE_PREFIX_PATH=<odb_install_directory>:$CMAKE_PREFIX_PATH

export PATH=<thrift_install_dir>/bin:$PATH
export PATH=<odb_install_directory>/bin:$PATH
```

Use the following instructions to build CodeCompass with CMake.

```bash
# Obtain CodeCompass source code.
git clone https://github.com/Ericsson/CodeCompass.git --origin upstream
cd CodeCompass

# Create build directory.
mkdir Build
cd Build

# Run CMake
cmake .. \
  -DCMAKE_INSTALL_PREFIX=<CodeCompass_install_dir> \
  -DDATABASE=<database_type> \
  -DCMAKE_BUILD_TYPE=<build_type> \
  -DLLVM_DIR=/usr/lib/llvm-10/cmake \
  -DClang_DIR=/usr/lib/cmake/clang-10

# To specify linker for building CodeCompass use
#   -DCODECOMPASS_LINKER=<path_to_linker>

# Build project.
make -j $(nproc)

# Copy files to install directory.
make install
```

## CMake variables
Besides the common CMake configuration variables you can set the database to be
used. The following table contains a few CMake variables which might be
relevant during compilation.

|       Variable       |                  Meaning                 |
| -------------------- | ---------------------------------------- |
| [`CMAKE_INSTALL_PREFIX`](http://cmake.org/cmake/help/v3.4/variable/CMAKE_INSTALL_PREFIX.html) | Install directory. |
| [`CMAKE_BUILD_TYPE`](http://cmake.org/cmake/help/v3.4/variable/CMAKE_BUILD_TYPE.html)| Specifies the build type. Supported values are **`Debug`**, **`Release`**, **`RelWithDebInfo`** and **`MinSizeRel`**. Default is **`RelWithDebInfo`** if not specified. |
| `CMAKE_CXX_COMPILER` | If the official repository of your Linux distribution doesn't contain a C++ compiler which supports C++14 then you can install one manually and set to use it. For more information see: ['Useful variables'](https://cmake.org/Wiki/CMake_Useful_Variables) |
| `DATABASE` | Database type. Possible values are **sqlite**, **pgsql**. The default value is `sqlite`. |
| `TEST_DB` | The connection string for the database that will be used when executing tests with `make test`. Optional. |
| `CODECOMPASS_LINKER` | The path of the linker, if the system's default linker is to be overridden. |
| `WITH_PLUGIN`/`WITHOUT_PLUGIN` | The names of the plugins to be built/skipped at build. Possible values are **cpp**, **cpp_reparse**, **dummy**, **git**, **metrics**, **search**. The `metrics` and `search` plugins are fundamental, they will be compiled even if not included. `WITH_PLUGIN` **cannot** be used together with `WITHOUT_PLUGIN`. Example: `-DWITH_PLUGIN="cpp;git"` This will compile the cpp, git, metrics and search plugins. |
| `WITH_AUTH` | The names of the authentication plugins to be compiled. Possible values are **plain** and **ldap**. `plain` **cannot** be skipped. Example: `-DWITH_AUTH="plain;ldap"`|
