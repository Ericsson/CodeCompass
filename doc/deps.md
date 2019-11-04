# Build Environment
We build CodeCompass under Linux. Currently, we are supporting Ubuntu 16.04 LTS
and Ubuntu 18.04 LTS. It is recommended to use a 64-bit operating system.

We also provide a Docker image that can be used as developer environment to
CodeCompass. See its usage at the [bottom](#docker) of this page.

# Dependencies
The following third-party tools are needed for building CodeCompass. These can
be installed from the official repository of the given Linux distribution.

- **`git`**: For fetching and managing CodeCompass source code.
- **`cmake`** and **`make`**: For building CodeCompass.
- **`g++`**: For compiling CodeCompass. A version which supports C++14 features
  is required. (Alternatively, you can compile with Clang.)
- **`libboost-all-dev`**: Boost can be used during the development.
- **`llvm-7-dev`**, **`libclang-7-dev`**: C++ parser uses LLVM/Clang for
  parsing the source code.
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
- **`libssl-dev`**: OpenSSL libs are required by Thrift.
- **`libgraphviz-dev`**: GraphViz is used for generating diagram visualizaions.
- **`libmagic-dev`**: For detecting file types.
- **`libgit2-dev`**: For compiling Git plugin in CodeCompass.
- **`npm`** (and **`nodejs-legacy`** for Ubuntu 16.04): For handling
  JavaScript dependencies for CodeCompass web GUI.
- **`ctags`**: For search parsing.
- **`libgtest-dev`**: For testing CodeCompass.  ***See [Known
  issues](#known-issues)!***

## Quick guide

The following command installs the packages except for those which have some
known issues.

#### Ubuntu 16.04 LTS

The standard Ubuntu Xenial package repository contains only LLCM/Clang version
6, which is not sufficient for CodeCompass, as at least version 7.0 is
required.  Therefore LLVM and Clang should be installed from the official LLVM
repositories:

```bash
sudo deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main
sudo deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main

sudo apt-get install git cmake make g++ libboost-all-dev \
  llvm-7-dev libclang-7-dev odb libodb-dev \
  default-jdk libssl-dev libgraphviz-dev libmagic-dev libgit2-dev ctags \
  libgtest-dev npm nodejs-legacy
```

#### Ubuntu 18.04 LTS

```bash
sudo apt-get install git cmake make g++ libboost-all-dev \
  llvm-7-dev libclang-7-dev odb libodb-dev \
  default-jdk libssl-dev libgraphviz-dev libmagic-dev libgit2-dev ctags \
  libgtest-dev npm
```

#### Database engine support

Depending on the desired database engines to be supported, the following
packages should be installed:

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

### Thrift
CodeCompass needs [Thrift](https://thrift.apache.org/) which provides Remote
Procedure Call (RPC) between the server and the client. Thrift is not part of
the official Ubuntu 16.04 LTS and 18.04 LTS repositories, but you can download
it and build from source:

```bash
# Download and uncompress Thrift:
wget "http://www.apache.org/dyn/mirrors/mirrors.cgi?action=download&filename=thrift/0.12.0/thrift-0.12.0.tar.gz" \
  -O thrift-0.12.0.tar.gz
tar -xvf ./thrift-0.12.0.tar.gz
cd thrift-0.12.0

# Ant is required for having Java support in Thrift.
sudo apt-get install ant

# Thrift can generate stubs for many programming languages. The configure
# script looks at the development environment and if it finds the environment
# for a given language then it'll use it. For example in the previous step npm
# was installed which requires NodeJS. If NodeJS can be found on your machine
# then the corresponding stub will also compile. If you don't need it then you
# can turn it off: ./configure --without-nodejs.
#
# In certain cases, installation may fail if development libraries for
# languages are not installed on the target machine. E.g. if Python is
# installed but the Python development headers are not, Thrift will unable to
# install. Python, PHP and such other Thrift builds are NOT required by
# CodeCompass.

./configure --prefix=<thrift_install_dir> --with-python=NO --with-php=NO

make install
```

### GTest/Googletest
The `libgtest-dev` package contains only the source files of GTest, but the
binaries are missing. You have to compile GTest manually.

#### Ubuntu 16.04 LTS
As further complications, under Ubuntu Xenial, the *install* instructions
are also missing from GTest's build system, so the target binaries
have to copied manually to the install location.

```bash
mkdir gtest
cp -R /usr/src/gtest/* ./gtest

cd gtest
mkdir build
cd build

cmake ..
make

mkdir -p <gtest_install_dir>/lib
cp libgtest.a libgtest_main.a <gtest_install_dir>/lib/
```

#### Ubuntu 18.04 LTS
```bash
mkdir gtest
cp -R /usr/src/googletest/* ./gtest

cd gtest
mkdir build
cd build

cmake .. -DCMAKE_INSTALL_PREFIX=<gtest_install_dir>
make install
```

# Build CodeCompass
The dependencies which are installed manually because of known issues have to
be seen by CMake build system:

```bash
export GTEST_ROOT=<gtest_install_dir>
export CMAKE_PREFIX_PATH=<thrift_install_dir>:$CMAKE_PREFIX_PATH
export PATH=<thrift_install_dir>/bin:$PATH
```

Use the following instructions to build CodeCompass with CMake.

```bash
# Obtain CodeCompass source code.
git clone https://github.com/Ericsson/CodeCompass.git
cd CodeCompass

# Create build directory.
mkdir build
cd build

# Run CMake
cmake .. \
  -DCMAKE_INSTALL_PREFIX=<CodeCompass_install_dir> \
  -DDATABASE=<database_type> \
  -DCMAKE_BUILD_TYPE=<build_type>

# Build project.
make -j<number_of_threads>

# Copy files to install directory.
make install
```

## CMake variables
Besides the common CMake configuration variables you can set the database to be
used. The following table contains a few CMake variables whic might be relevant
during compilation.

| Variable | Meaning |
| -------- | ------- |
| [`CMAKE_INSTALL_PREFIX`](http://cmake.org/cmake/help/v3.0/variable/CMAKE_INSTALL_PREFIX.html) | Install directory. |
| [`CMAKE_BUILD_TYPE`](http://cmake.org/cmake/help/v3.0/variable/CMAKE_BUILD_TYPE.html)| Specifies the build type on single-configuration generators. Possible values are empty, **`Debug`**, **`Release`**. |
| `CMAKE_CXX_COMPILER` | If the official repository of your Linux distribution doesn't contain a C++ compiler which supports C++14 then you can install one manually and set to use it. For more information see: https://cmake.org/Wiki/CMake_Useful_Variables |
| `DATABASE` | Database type. Possible values are **sqlite**, **pgsql**. The default value is `sqlite`. |
| `TEST_DB` | The connection string for the database that will be used when executing tests with `make test`. |

# Docker

Docker related scripts and config files are under
[/scripts/docker](/scripts/docker). After navigating to this directory you can
use the build scripts with the commands below.

:warning: When using Docker to build CodeCompass it is very important to have
the following filesystem layout:

```
...
 `-CodeCompass
   |-CodeCompass   # Source code from Git.
   | `-scripts
   |   `-docker
   |-build         # CMake runs here.
   |-install       # CodeCompass goes here.
   `-workspace     # Parsed projects' workspace directory.
```

The scripts assume this layout. The `build` and `install` directories will be
generated to the parent directory of `CodeCompass` directory containing the
source code.

1. Build the development environment image. The tag name is important!
```
docker build -t codecompass-dev .
```
2. Build CodeCompass.
```
docker run \
  --env DATABASE=pgsql --env BUILD_TYPE=Release \
  --volume /path/to/host/CodeCompass:/CodeCompass \
  codecompass-dev <make flags>
```
The `<make flags>` is added to the `make` command when building CodeCompass.
Here you can set the number of build jobs or `install`.
