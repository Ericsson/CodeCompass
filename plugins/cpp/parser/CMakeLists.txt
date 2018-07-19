find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# Set the runtime path of LLVM libraries to be in the rpath of the build.
# This only applies for the current folder's scope (C++ Parser plugin).
mark_run_env_path(LLVM LD_LIBRARY_PATH "${LLVM_BUILD_LIBRARY_DIR}")
set_rpath_with_libraries(LLVM)

llvm_map_components_to_libnames(llvm_libs support core irreader)

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
  src/manglednamecache.cpp
  src/ppincludecallback.cpp
  src/ppmacrocallback.cpp
  src/relationcollector.cpp
  src/doccommentformatter.cpp)

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
  clang
  LLVMMCParser
  LLVMOption
  ${llvm_libs})

target_compile_options(cppparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS cppparser DESTINATION ${INSTALL_PARSER_DIR})

# Install Clang additional files. These are needed so the running Clang tool
# environment can find standard headers.
install(DIRECTORY
  ${LLVM_LIBRARY_DIRS}/clang
  DESTINATION "${INSTALL_LIB_DIR}")
