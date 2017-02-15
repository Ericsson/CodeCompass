# Operating system
We build CodeCompass under Linux. It is recommended to use a 64-bit operating system.

# Disk requirements
It is recommended to have at least 10 GB of free space and at least 2GB RAM before you begin for convenient work.

Disk space requirements of the CodeCompass build:

|  | *64-bit* |
| -------- | -------- |
| Dependencies   | 9 GB   |
| CodeCompass   | 1 GB   |
| **Total**   | **10 GB**   |

(Plus additional space for parsed projects)

# Dependencies
The CodeCompass should be built with specific versions of the libraries and tools it depends on. To ease the build process this environment can be obtained by the use of the `scripts/build_deps.sh` script.

# Directory structure
Inside the `cc` directory (which can be in your home directory), you will see the following subdirectories at the end of the installation:
```
/home/username/cc
  |  (the development directory for CodeCompass)
  |
  +- CodeCompass
  |     (from git, source and build directory)
  +- CodeCompass-deps
  |     (for dependencies, can be shared between multiple compilations)
  \- CodeCompass-install
        (install directory of CodeCompass)
```
# Building dependencies and CodeCompass

## Dependencies

Use the following instructions to build the dependencies
```bash
# create a directory for CodeCompass
mkdir cc
cd cc

# Clone the CodeCompass revision from your fork of the repository
git clone https://github.com/<username>/CodeCompass

# build the dependencies (long, takes 3-4 hours on an average computer)
cd CodeCompass/scripts
./build_deps.sh
```
This compiles all libraries used by our project and all tools required to compile our code into the `CodeCompass-deps` directory.

Also creates a file named `env.sh` in the `CodeCompass-install` directory, that sets the compilation environment (compiler, include paths, library paths) and can be loaded using the source command.

## Build CodeCompass with the build script

Use the following instructions to build CodeCompass with cmake.
```bash
# Load the CodeCompass build environment
# After the environment is loaded, your prompt will have (cc) prefix
source ../CodeCompass-install/env.sh

# Create build directory
mkdir -p cc/CodeCompass/build
cd cc/CodeCompass/build

# Run cmake
cmake .. -DCMAKE_INSTALL_PREFIX=/home/<username>/cc/CodeCompass-install -DDATABASE=<database_type> -DCMAKE_BUILD_TYPE=<build_type> -DCMAKE_CXX_COMPILER=/home/<username>/cc/CodeCompass-deps/bin/g++

# Build project
make -j<numb_of_threads>

# Copy files to install directory (which in this case is /home/<username>/cc/CodeCompass-install)
make install
```

## CMake variables
| Variable | *Meaning* |
| -------- | -------- |
|  *CMAKE_INSTALL_PREFIX*  | Install directory used by install. For more information see: https://cmake.org/cmake/help/v3.0/variable/CMAKE_INSTALL_PREFIX.html|
|  *CMAKE_BUILD_TYPE*  |  Specifies the build type on single-configuration generators. Possible values are empty, **Debug**, **Release**. For more information see: https://cmake.org/cmake/help/v3.0/variable/CMAKE_BUILD_TYPE.html|
|  *CMAKE_CXX_COMPILER*  | The compiler used for C++ files. It doesn't necessary. It has to support C++11 if you give one. For more information see: https://cmake.org/Wiki/CMake_Useful_Variables |
|  *DATABASE*  | ODB database type. Possible values are **sqlite**, **pgsql**. The default value is sqlite. |

# Development
After modifying some files, use simply make to rebuild:
```makefile
make -j<num_of_threads>
```
Or a *make install* to update the installation files as well:
```
make install
```

# Tested on:
* Ubuntu 14.04 (x86_64)

# :exclamation: Known issues::exclamation:
## Thrift compilation error: 
```
g++: error: /home/<username>/cc/CodeCompass-deps/lib64/libboost_unit_test_framework.a: No such file or directory
```
Solution (http://stackoverflow.com/questions/11310434/errorno-rule-to-make-target-usr-local-lib-libboost-unit-test-framework-a): 
```
cp /home/<username>/cc/CodeCompass-deps/lib/libboost_unit_test_framework.a /home/<username>/cc/CodeCompass-deps/lib64
```
