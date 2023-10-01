include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include)

add_library(cppmetricsparser SHARED
  src/cppmetricsparser.cpp)

target_compile_options(cppmetricsparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS cppmetricsparser DESTINATION ${INSTALL_PARSER_DIR})
