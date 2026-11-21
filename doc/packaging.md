# Packaging
Before running or installing any package verify its checksum:
```
sha256sum <package file>
```

# AppImage

## Running CodeCompass AppImage
Set execution permissions:
```
chmod +x ./CodeCompass.AppImage
```

Run AppImage:
```
./CodeCompass.AppImage
```

AppImage Usage:
```
./CodeCompass.AppImage parser <parser arguments>
```
```
./CodeCompass.AppImage webserver <webserver arguments>
```
```
./CodeCompass.AppImage logger <logger arguments>
```

## Building CodeCompass AppImage

### appimagetool
To build AppImages, we need to use `appimagetool`.
1. Download `appimagetool-x86_64.AppImage` from https://github.com/AppImage/AppImageKit/releases
2. Place `appimagetool-x86_64.AppImage` into a directory which is added to the `PATH` environmental variable.
3. Set execution permissions: ```chmod +x ./appimagetool-x86_64.AppImage```
4. Verify `appimagetool-x86_64.AppImage` is runnable from any directory: `which appimagetool-x86_64.AppImage`

### Building AppImage
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
make install -j $(nproc)
```

Build AppImage:
```
make appimage 
```

The built package will be located in `build/packaging/`.

## Build options
The built package can be customized by using CMake variables.

|       Variable       |                  Meaning                 |
| -------------------- | ---------------------------------------- |
| `PACKAGE_VERSION` | Version of the package for the manifest file. If not specified, version `1.0` will be used. |

