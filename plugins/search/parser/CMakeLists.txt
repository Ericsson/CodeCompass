include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/model/include/model
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${PROJECT_BINARY_DIR}
  ${PLUGIN_DIR}/indexer/gen-cpp
  ${PLUGIN_DIR}/indexer/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_library(searchparser SHARED src/searchparser.cpp)
target_link_libraries(searchparser
  util
  magic
  indexerservice)

target_compile_options(searchparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS searchparser DESTINATION ${INSTALL_PARSER_DIR})
