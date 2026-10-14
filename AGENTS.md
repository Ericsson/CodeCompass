# AGENTS.md

This file provides guidance to AI coding agents when working with code in this repository.

## Overview

CodeCompass is a pluginable code comprehension tool. A C++17 backend parses
projects into an SQL database (via ODB) and a webserver serves the results over
Thrift RPC to a web frontend. Two frontends exist: the legacy Dojo-based
`webgui/` (served at `/`) and the React/Next.js `webgui-new/` (served at
`/new`).

## Build

Out-of-source CMake build (see `doc/deps.md` for dependencies: LLVM/Clang 15,
ODB built via build2, Thrift, Boost, GTest, etc.):

```bash
mkdir build && cd build
cmake .. \
  -DCMAKE_INSTALL_PREFIX=<install_dir> \
  -DDATABASE=<sqlite|pgsql> \
  -DCMAKE_BUILD_TYPE=<Debug|Release|RelWithDebInfo> \
  -DLLVM_DIR=/usr/lib/llvm-15/cmake \
  -DClang_DIR=/usr/lib/cmake/clang-15 \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
  -DTEST_DB="<connection_string>"     # optional, enables functional tests
make -j $(nproc)
make install
```

Notable CMake variables:
- `DATABASE` (default `sqlite`) is a **compile-time** choice: it drives ODB code
  generation and defines `DATABASE_SQLITE`/`DATABASE_PGSQL`. Switching backend
  requires a separate build directory.
- `WITH_PLUGIN` / `WITHOUT_PLUGIN` (mutually exclusive), e.g.
  `-DWITH_PLUGIN="cpp;git"`. The `search` plugin is always built.
- `WITH_AUTH="plain;ldap"` selects authentication plugins.
- `CODECOMPASS_LINKER` overrides the default `gold` linker.

ODB/GTest from a custom prefix need `CMAKE_PREFIX_PATH`, `PATH` (for the `odb`
binary) and `GTEST_ROOT` set before running CMake.

## Tests

Tests are GTest binaries registered with CTest, living in `plugins/<name>/test/`.
Functional tests are only generated when `TEST_DB` is set at configure time, and
its scheme must match `DATABASE` (e.g. `sqlite:database=/path/test.sqlite` or
`pgsql:host=localhost;port=5432;user=...;password=...;database=...`).

**Tests run against the installed binaries**: each test first builds a small
sample project from `test/sources/` with CMake, then runs
`${CMAKE_INSTALL_PREFIX}/bin/CodeCompass_parser` on it, then runs GTest queries
against `TEST_DB`. So always `make install` before `make test`.

```bash
make install && make test ARGS=-V      # all tests (as in CI)
ctest -V -R cppparser                  # single CTest test (from build dir)
```

Registered test names: `cppparser`, `cppservice`, `cppmetricsparser`,
`cppmetricsservice`, `pythonparser`, `pythonservice`. To filter individual
GTest cases, pass `--gtest_filter=...` after the three positional args the test
binary expects (build command, parser command, DB string) — easiest to copy
them from `ctest -V -N` / `CTestTestfile.cmake`.

## Running

```bash
CodeCompass_parser -w <workspace> -n <project_name> -d <conn_string> \
  -i <compile_commands.json> -i <source_dir> [-j N] [--force|--dry-run]
CodeCompass_webserver -w <workspace> -p <port>
```

The workspace must not be inside the parsed source tree. Without `--force`,
re-parsing an existing project is incremental (only C++ and metrics parsers
truly support it). `CodeCompass_logger` produces a compilation database for
non-CMake builds. See `doc/usage.md`.

## Frontend (webgui-new)

```bash
cd webgui-new
npm install
./thrift-codegen.sh --thrift-source ../   # regenerate TS stubs from all *.thrift
npm run dev                               # dev server on :3000, proxies to BACKEND_URL
npm run lint                              # run in CI
```

Configure `BACKEND_URL`, `DEVSERVER_PORT`, `PUBLIC_URL` in `.env.local`. The
generated Thrift TS stubs land in `webgui-new/generated/`; rerun codegen after
changing any `.thrift` file.

## Architecture

Everything beyond the core is a **plugin loaded at runtime as a shared library**
with `extern "C"` entry points:

- **Parser plugins** (`plugins/<p>/parser`) subclass `cc::parser::AbstractParser`
  (`parser/include/parser/abstractparser.h`) and export `getOptions()` and
  `make(ParserContext&)`. Installed to `lib/parserplugin/`; `CodeCompass_parser`
  (`parser/src/parser.cpp`) dlopens every library there, runs
  `markModifiedFiles()` / `cleanupDatabase()` for incremental parsing, then
  `parse()` (parsers returning `isDatabaseIndexRequired()` run after DB indexes
  are created). Inputs (`-i`) are offered to every parser, which decide what
  they consume.
- **Service plugins** (`plugins/<p>/service`, `service/*`) implement Thrift
  services and export `getOptions()` and `registerPlugin(ServerContext,
  PluginHandler*)` (see `plugins/dummy/service/src/plugin.cpp`). Installed to
  `lib/serviceplugin/`; `CodeCompass_webserver` (`webserver/`, embedded Mongoose
  HTTP server) loads them and routes requests per project in the workspace.
- **Auth plugins** (`webserver/authenticators/`) installed to `lib/authplugin/`.

Core modules:
- `model/` — core ODB persistent classes (`File`, `FileContent`, `BuildAction`,
  …). Plugins add their own `model/` directories; headers are run through ODB
  via `generate_odb_files()` / `add_odb_library()` (`Functions.cmake`),
  producing `*-odb.hxx/.cxx` and `.sql` schema files (installed to
  `share/codecompass/sql`) in the build tree.
- `service/` — shared Thrift APIs: `project.thrift` / `common.thrift` (files,
  build logs), `language.thrift` (the common interface that language plugins
  such as `cpp` and `python` implement), `workspace`, `plugin`,
  `authentication`, and the LSP bridge.
- `util/` — DB connection/transaction helpers (`odbtransaction.h`, `dbutil.h`),
  logging, thread pool, graph (GraphViz) helpers, dynamic library loading.
- `logger/` — `CodeCompass_logger` (LD_PRELOAD-based build command logger).

A typical plugin (`plugins/dummy` is the template) has `model/`, `parser/`,
`service/` (with a `.thrift` file), `test/` and `webgui/` (JS/CSS/images for the
legacy GUI, installed by `install_webplugin()`; the new GUI lives entirely in
`webgui-new/src`). Plugin-to-plugin model dependencies are passed as extra args
to `generate_odb_files()`.

The Python plugin's parser is itself written in Python (`plugins/python/parser/pyparser/`)
and embedded via Boost.Python; see `doc/pythonplugin.md`.

## Coding conventions (C++, from `doc/coding_conventions.md`)

- 80-column lines, 2-space indent, no tabs; braces on their own line (Allman);
  namespaces not indented, closed with `} // name`.
- Namespaces: two levels, `cc::<module>` (e.g. `cc::parser`, `cc::model`);
  services nest further (`cc::service::language`).
- Naming: files lowercase without dashes (`.h`/`.cpp`); types `CamelCase`;
  functions `camelCase`; members `_camelCase`; parameters `camelCase_`;
  smart-pointer typedefs end in `Ptr`.
- `int* ptr`, `const T& ref` — qualifier attached to the type.
- Include guards `CC_<MODULE>_<FILE>_H`. Public headers under
  `include/<submodule>/` included with `<>`; private headers in `src/` with `""`.
  Include order: std, Boost, other third-party (LLVM/ODB/…), project headers.
- One `public`/`protected`/`private` section each, in that order; friends first.
- Never `using namespace` in headers.
