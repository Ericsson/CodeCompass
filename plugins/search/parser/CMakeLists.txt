include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${CMAKE_BINARY_DIR}/model/include
  ${PLUGIN_BINARY_DIR}/indexer/gen-cpp
  ${PLUGIN_DIR}/indexer/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS}
  ${LIBMAGIC_INCLUDES})

add_library(searchparser SHARED src/searchparser.cpp)
target_link_libraries(searchparser
  util
  ${LIBMAGIC_LIBRARIES}
  indexerservice)

target_compile_options(searchparser PUBLIC -Wno-unknown-pragmas)

if(APPLE)
  # Use Linux-like linking behaviour, dynamic libraries may contain references without implementation.
  # searchparser references headers from the parser without implementation
  set_target_properties(searchparser PROPERTIES LINK_FLAGS
      "-undefined dynamic_lookup")
endif(APPLE)

install(TARGETS searchparser DESTINATION ${INSTALL_PARSER_DIR})
