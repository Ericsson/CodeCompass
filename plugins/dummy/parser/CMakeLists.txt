include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include)

set(SOURCES src/dummyparser.cpp)

add_library(dummyparser SHARED ${SOURCES})

install(TARGETS dummyparser DESTINATION ${INSTALL_PARSER_DIR})
