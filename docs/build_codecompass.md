# :computer: Operating system
We build CodeCompass under Linux. It is recommended to use a 64-bit operating system.

# :exclamation: Disk requirements
It is recommended to have at least 16-32 GB of free space and at least 2GB RAM before you begin for convenient work.


Disk space requirements of the CodeCompass build:

|              | *32-bit*  | *64-bit*  |
| ------------ | --------- | --------  |
| Dependencies | 10 GB     | 11 GB     |
| CodeCompass  | 1 GB      | 2 GB      |
| **Total**    | **11 GB** | **13 GB** |

(Plus additional space for parsed projects)

# :exclamation: Dependencies
The trunk should be built with specific versions of the libraries and tools it depends on. To ease the build process this environment can be obtained by the use of the build_deps script.

## Ubuntu
Building the tools require a gcc compiler and the package called gcc-multilib in Debian based systems (e.g. Mint, Ubuntu). (Note that each gcc version has its own multilib package. If you have installed other version gcc packages and set a different version as default you may need to install gcc-X.Y-multilib package.) Similarly, the package python-dev should match the version installed in the system.

On a fresh install, the following command should install every necessary package:
```bash
sudo apt-get install gcc-multilib g++-multilib python-dev zlib1g-dev libssl-dev texinfo
```

## openSUSE

On openSUSE, you need some development tools (such as gcc) as well as the development packages for the used libraries. On a fresh install, issuing the next command should do the job.
```bash
sudo zypper install make gcc glibc-devel-32bit python-devel zlib-devel patch libopenssl-devel subversion texinfo
```

# :file_folder: Directory structure
Inside the cc directory, you will see the following subdirectories at the end of the installation:
```
cc
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

## :exclamation: Dependencies

Use the following instructions to build the dependencies
```bash
# create a directory for CodeCompass
mkdir cc
cd cc

# Clone the CodeCompass revision from the repository
git clone https://github.com/Ericsson/CodeCompass.git

# build the dependencies (long, takes 3-4 hours on an average computer)
CodeCompass/scripts/build_deps.sh
```
This compiles all libraries used by our project and all tools required to compile our code into the CodeCompass-deps directory.

Also creates a file named *env.sh* in the *CodeCompass-install* directory, that sets the compilation environment (compiler, include paths, library paths) and can be loaded using the source command.
## :zap: CodeCompass

Use the following instructions to build CodeCompass
```bash
cd CodeCompass

# load the CodeCompass build environment
# (build_cc.sh loads this but the environment will be needed
#  throughout development anyway later)
source ../CodeCompass-install/env.sh

# after the environment is loaded, your prompt will have (cc) prefix

# build the CodeCompass with pgsql database engine (long, takes 20 mins on an average computer)
scripts/build_cc.sh -b -i -o
```

CodeCompass is built using autotools. *build_cc.sh* first runs *./autogen.sh* to initialize the autotools system, then it runs *./configure* to setup the build, then it builds CodeCompass using make, finally it creates an installation using make install.

## :white_check_mark: Running CodeCompass tests
Use the check make target to run the tests. The result will be displayed. All tests should pass before you commit.
```
make check
```

# :computer: Development
After modifying some files, use simply make to rebuild:
```
make
```
Or a *make install* to update the installation files as well:
```
make install
```

*build_cc.sh* supports some advanced build options. See help for details:
```
scripts/build_cc.sh --help
```

# :heavy_check_mark: Tested on:
* Ubuntu 12.04 (x86_64)
* Ubuntu 14.04 (x86)
* Ubuntu 14.04 (x86_64) 
