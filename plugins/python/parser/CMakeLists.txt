include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include)

add_library(pythonparser SHARED
  src/pythonparser.cpp)

target_compile_options(pythonparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS pythonparser DESTINATION ${INSTALL_PARSER_DIR})
