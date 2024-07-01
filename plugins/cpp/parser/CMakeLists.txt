# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Check whether RTTI is on for LLVM
message(STATUS "Using RTTI for LLVM: ${LLVM_ENABLE_RTTI}")
if(NOT LLVM_ENABLE_RTTI)
    message(SEND_ERROR "RTTI is required for LLVM")
endif()

# Check LLVM version
if (${LLVM_PACKAGE_VERSION} VERSION_LESS "10")
    message(SEND_ERROR "Loaded LLVM version must be at least 10.0, ${LLVM_PACKAGE_VERSION} found.")
endif()

# Add LLVM to the library path
mark_as_run_env_path(LD_LIBRARY_PATH "${LLVM_BUILD_LIBRARY_DIR}")

# Find Clang
find_package(Clang REQUIRED CONFIG)
message(STATUS "Using ClangConfig.cmake in: ${Clang_DIR}")

include_directories(
  include
  ${CMAKE_SOURCE_DIR}/parser/include
  ${CMAKE_SOURCE_DIR}/util/include
  ${CMAKE_SOURCE_DIR}/model/include
  ${PLUGIN_DIR}/model/include)

include_directories(SYSTEM
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS})

link_directories(${LLVM_LIBRARY_DIRS})

add_definitions(${LLVM_DEFINITIONS})

add_library(cppparser SHARED
  src/cppparser.cpp
  src/symbolhelper.cpp
  src/entitycache.cpp
  src/ppincludecallback.cpp
  src/ppmacrocallback.cpp
  src/relationcollector.cpp
  src/doccommentformatter.cpp
  src/diagnosticmessagehandler.cpp
  src/nestedscope.cpp)

target_link_libraries(cppparser
  cppmodel
  model
  clangTooling
  clangFrontend
  clangDriver
  clangSerialization
  clangParse
  clangSema
  clangAnalysis
  clangEdit
  clangAST
  clangLex
  clangBasic
  clangIndex
  clang)

target_compile_options(cppparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS cppparser DESTINATION ${INSTALL_PARSER_DIR})

# Install Clang additional files
install(DIRECTORY
        ${LLVM_LIBRARY_DIRS}/clang
        DESTINATION "${INSTALL_LIB_DIR}")
