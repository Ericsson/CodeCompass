llvm_map_components_to_libnames(llvm_libs support core irreader)

include_directories(
  include
  ${CMAKE_SOURCE_DIR}/parser/include
  ${CMAKE_SOURCE_DIR}/util/include
  ${CMAKE_SOURCE_DIR}/model/include
  ${cxx_PLUGIN_DIR}/model/include
  ${LLVM_INCLUDE_DIRS})

add_definitions(${LLVM_DEFINITIONS})

add_library(cxxparser SHARED
  src/cxxparser.cpp
  src/symbolhelper.cpp)

target_link_libraries(cxxparser
  cxxmodel
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

target_compile_options(cxxparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS cxxparser DESTINATION ${INSTALL_PARSER_DIR})
