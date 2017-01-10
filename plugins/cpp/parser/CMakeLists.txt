llvm_map_components_to_libnames(llvm_libs support core irreader)

include_directories(
  include
  ${CMAKE_SOURCE_DIR}/parser/include
  ${CMAKE_SOURCE_DIR}/util/include
  ${CMAKE_SOURCE_DIR}/model/include
  ${PLUGIN_DIR}/model/include)

include_directories(SYSTEM
  ${LLVM_INCLUDE_DIRS})

link_directories(${LLVM_LIBRARY_DIRS})

add_definitions(${LLVM_DEFINITIONS})

add_library(cppparser SHARED
  src/cppparser.cpp
  src/symbolhelper.cpp
  src/manglednamecache.cpp
  src/idcache.cpp
  src/ppincludecallback.cpp
  src/ppmacrocallback.cpp
  src/relationcollector.cpp
  src/doccommentformatter.cpp
  src/pointeranalysiscollector.cpp)

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
