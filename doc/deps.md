# Operating system
We build CodeCompass under Linux. Currently, we are supporting Ubuntu 16.04 LTS.
It is recommended to use a 64-bit operating system.

# Dependencies
CodeCompass uses some third-party dependencies. These can be installed from the
official repository of the given Linux distribution. On Ubuntu, the following
packages are necessary for building CodeCompass:

- **`git`**: For fetching and managing CodeCompass source code.
- **`cmake`** and **`make`**: For building CodeCompass.
- **`g++`**: For compiling CodeCompass. A version which supports C++14 features
  is required. (Alternatively, you can compile with Clang.)
- **`libboost-all-dev`**: Boost can be used during the development.
- **`llvm-7.0`**, **`clang-7.0`**: For compiling CodeCompass with Clang
  instead of G++.
- **`llvm-7.0-dev`**, **`libclang-7.0-dev`**: C++ parser uses LLVM/Clang for
  parsing the source code. Version 7.0 or newer is required. Clang 7 is not yet
  released, so the project must be compiled manually from source.
  ***See [Known issues](#known-issues)!***
- **`libsqlite3-dev`**: SQLite library in case SQLite database system is used.
- **`postgresql-server-dev-<version>`**: PostgreSQL server in case PostgreSQL
  database system is used.
- **`odb`**: For persistence ODB can be used which is an Object Relation
  Mapping (ORM) system.
- **`openjdk-8-jdk`**: For search parsing CodeCompass uses an indexer written
  in Java.
- **`libssl-dev`**: OpenSSL libs are required by Thrift.
- **`libgraphviz-dev`**: GraphViz is used for generating diagram visualitaions.
- **`libmagic-dev`**: For detecting file types.
- **`libgit2-dev`**: For compiling Git plugin in CodeCompass.
- **`npm`**, and **`nodejs-legacy`** (for 16.04) or **`nodejs`** (for 18.04):
  For handling JavaScript dependencies for CodeCompass web GUI.
- **`ctags`**: For search parsing.
- **`libgtest-dev`**: For testing CodeCompass.
  ***See [Known issues](#known-issues)!***

The following command installs the packages except for those which have some
known issues:
```bash
sudo apt-get install git cmake make g++ libboost-all-dev openjdk-8-jdk \
  libssl-dev libgraphviz-dev libmagic-dev libgit2-dev npm ctags \
  libgtest-dev

# For Ubuntu 16.04 "Xenial Xerus" LTS:
sudo apt-get install nodejs-legacy

# For Ubuntu 18.04 "Bionic Beaver" LTS:
sudo apt-get install nodejs
```

## Known issues
Some third-party tools are present in the distribution's package manager in a
way that they are eiter incompatible with each other or not available as a
package, thus can't be used to create your CodeCompass installation.

:warning: Building and installing from source code to the system is a dangerous
operation - outside package managers, the wrong binaries can end up being used
by other processes which could, in extreme cases, make the system very hard or
impossible to recover. **Please do NOT add a `sudo` in front of any `make` or
other commands below, unless *explicitly* specified!**

### Thrift
CodeCompass needs [Thrift](https://thrift.apache.org/) which provides Remote
Procedure Call (RPC) between the server and the client. Thrift is not part of
the official Ubuntu 16.04 LTS repositories, but you can download it and build
from source:

```bash
# Thrift may require yacc and flex as dependency:
sudo apt-get install byacc flex

# Download and uncompress Thrift:
wget "http://www.apache.org/dyn/mirrors/mirrors.cgi?action=download&filename=thrift/0.10.0/thrift-0.10.0.tar.gz" \
  -O thrift-0.10.0.tar.gz
tar -xvf ./thrift-0.10.0.tar.gz
cd thrift-0.10.0

./configure --prefix=<thrift_install_dir> --with-python=NO --with-php=NO
# Thrift can generate stubs for many programming languages. The configure script
# looks at the development environment and if it finds the environment for a
# given language then it'll use it. For example in the previous step npm was
# installed which requires NodeJS. If NodeJS can be found on your machine then
# the corresponding stub will also compile. If you don't need it then you can
# turn it off: ./configure --without-nodejs.
#
# In certain cases, installation may fail if development libraries for languages
# are not installed on the target machine. E.g. if Python is installed but the
# Python development headers are not, Thrift will unable to install.
# Python, PHP and such other Thrift builds are NOT required by CodeCompass.

make install
```

### LLVM/Clang
In Ubuntu 16.04 LTS the LLVM/Clang has some packaging issues, i.e. some libs
are searched in `/usr/lib` however the package has these in
`/usr/lib/llvm-3.8/lib` (see
[`http://stackoverflow.com/questions/38171543/error-when-using-cmake-with-llvm`](http://stackoverflow.com/questions/38171543/error-when-using-cmake-with-llvm)
. This problem causes an error when emitting `cmake` command during CodeCompass
build. A solution would be to download a prebuilt package from the LLVM/Clang
webpage but another issue is that the prebuilt packages don't use runtime type
informations (RTTI) which is needed for CodeCompass. Clang needs to be compiled
with RTTI manually.

```bash
# If you want Clang's diagnostic output to have colours, install the following.
sudo apt-get install libtinfo-dev

wget http://releases.llvm.org/7.0.0/llvm-7.0.0.src.tar.xz
tar -xvf llvm-7.0.0.src.tar.xz
rm llvm-7.0.0.src.tar.xz
mv llvm-* llvm
cd llvm/tools
wget http://releases.llvm.org/7.0.0/cfe-7.0.0.src.tar.xz
tar -xvf cfe-7.0.0.src.tar.xz
rm cfe-7.0.0.src.tar.xz
mv cfe-* clang
cd ../..

mkdir build
cd build
cmake -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_RTTI=ON \
  -DCMAKE_INSTALL_PREFIX=<clang_install_dir> \
  ../llvm
# This make step takes a while. If you have more CPUs then you can compile on
# several threads with -j<number_of_threads> flag.
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

cmake .. \
  -DCMAKE_INSTALL_PREFIX=<gtest_install_dir>
make install
```

# Build CodeCompass
The dependencies which are installed manually because of known issues have to
be seen by CMake build system:

```bash
export GTEST_ROOT=<gtest_install_dir>
export CMAKE_PREFIX_PATH=<thrift_install_dir>:$CMAKE_PREFIX_PATH
export CMAKE_PREFIX_PATH=<clang_install_dir>:$CMAKE_PREFIX_PATH
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
