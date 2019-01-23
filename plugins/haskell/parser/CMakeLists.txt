include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include)

add_library(haskellparser SHARED
  src/haskellparser.cpp)

target_compile_options(haskellparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS haskellparser DESTINATION ${INSTALL_PARSER_DIR})
