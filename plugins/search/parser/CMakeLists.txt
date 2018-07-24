map_model_include_directories(${PROJECT_SOURCE_DIR}/model/include)
include_directories(${ODB_MODEL_INCLUDE_DIRECTORIES})

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${PLUGIN_DIR}/indexer/include
  ${PLUGIN_BINARY_DIR}/indexer/gen-cpp)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_library(searchparser SHARED src/searchparser.cpp)
target_link_libraries(searchparser
  util
  magic
  indexerservice)

target_compile_options(searchparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS searchparser DESTINATION ${INSTALL_PARSER_DIR})
