# `.deb` package (For Debian/Ubuntu systems)

## Installing a CodeCompass `.deb` package
Before installing a package, verify its checksum:
```
sha256sum codecompass.deb
```

### Installing a package
```
sudo dpkg -i codecompass.deb
```

### Removing package
```
sudo apt remove codecompass
```

## Package structure
When a user installs a CodeCompass deb package, all the files will be placed under `/usr/share/codecompass/`.
A few symlinks are created for the CodeCompass commands:
```
/usr/bin/CodeCompass_logger -> /usr/share/codecompass/bin/CodeCompass_logger
/usr/bin/CodeCompass_parser -> /usr/share/codecompass/bin/CodeCompass_parser
/usr/bin/CodeCompass_webserver -> /usr/share/codecompass/bin/CodeCompass_webserver
```

Moreover, CodeCompass requires some dependencies which are no longer available in standard repositories such as `ODB`.
In order to resolve this, the runtime dependencies will be included in the deb package as shared objects. These dependencies will be installed under `<CodeCompass_install_dir>/lib/deps/`.

Finally, a deb package is created by making a copy of the entire `<CodeCompass_install_dir>`.

## Building a `.deb` package
First, we need to build CodeCompass.

Create a build folder:
```
mkdir build
cd build
```

To enable packaging, run CMake with `-DENABLE_PACKAGING=1`:
```
cmake .. \
  -DCMAKE_INSTALL_PREFIX=<CodeCompass_install_dir> \
  -DDATABASE=<database_type> \
  -DCMAKE_BUILD_TYPE=<build_type> \
  -DLLVM_DIR=/usr/lib/llvm-11/cmake \
  -DClang_DIR=/usr/lib/cmake/clang-11 \
  -DENABLE_PACKAGING=1
```

Build and install CodeCompass:
```
make -j $(nproc)
make install
```

Build the deb package:
```
make deb_package
```

The built package will be located in `build/packaging/`.

## Build options
The built package can be customized by using CMake variables.

|       Variable       |                  Meaning                 |
| -------------------- | ---------------------------------------- |
| `PACKAGE_FILE_NAME` | File name of the resulting `.deb` package. Standard package notation: `<project>_<version>-<package revision>.deb`. If not specified, the package will be named `codecompass-dev.deb`. |
| `PACKAGE_VERSION` | Version of the package for the manifest file. If not specified, version `1.0` will be used. |
| `PACKAGE_DEPENDS` | List of package dependencies. Example: `graphviz, libcairo2 (>= 1.6.0), xdg-utils`. If not specified, the package will have no dependencies. |

