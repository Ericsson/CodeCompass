include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${PROJECT_SOURCE_DIR}/plugins/compilationdatabase/model/include
  ${LLVM_INCLUDE_DIRS}
)

set(SOURCES src/compilation_database_parser.cpp)

llvm_map_components_to_libnames(llvm_libs
  mc
  mcparser
  option
  core
  irreader)

add_library(compilationdatabaseparser SHARED ${SOURCES})
target_link_libraries(compilationdatabaseparser
  compilationdatabasemodel
  clangTooling
  clangFrontend
  clangSerialization
  clangDriver
  clangParse
  clangSema
  clangEdit
  clangAnalysis
  clangLex
  clangAST
  clangBasic
  ${llvm_libs}
)

install(TARGETS compilationdatabaseparser
	LIBRARY DESTINATION ${INSTALL_LIB_DIR}
	DESTINATION ${INSTALL_PARSER_DIR})