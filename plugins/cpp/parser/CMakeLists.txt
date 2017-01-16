llvm_map_components_to_libnames(llvm_libs support core irreader)

include_directories(
  include
  ${CMAKE_SOURCE_DIR}/parser/include
  ${CMAKE_SOURCE_DIR}/util/include
  ${CMAKE_SOURCE_DIR}/model/include
  ${PLUGIN_DIR}/model/include
  ${LLVM_INCLUDE_DIRS})

link_directories(${LLVM_LIBRARY_DIRS})

add_definitions(${LLVM_DEFINITIONS})

add_library(cppparser SHARED
  src/cppparser.cpp
  src/symbolhelper.cpp
  src/manglednamecache.cpp
  src/ppincludecallback.cpp
  src/ppmacrocallback.cpp
  src/relationcollector.cpp)

target_link_libraries(cppparser
  cppmodel
  model
  clangTooling
  clangFrontendTool
  clangFrontend
  clangDriver
  clangSerialization
  clangCodeGen
  clangParse
  clangSema
  clangStaticAnalyzerFrontend
  clangStaticAnalyzerCheckers
  clangStaticAnalyzerCore
  clangAnalysis
  clangARCMigrate
  clangRewrite
  clangRewriteFrontend
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
