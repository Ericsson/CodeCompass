include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include)

add_library(dummyparser SHARED
  src/dummyparser.cpp)

install(TARGETS dummyparser DESTINATION ${INSTALL_PARSER_DIR})
