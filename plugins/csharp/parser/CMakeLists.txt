add_subdirectory(src_csharp)

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${CMAKE_BINARY_DIR}/model/include)

add_library(csharpparser SHARED
  src/csharpparser.cpp)
  
target_link_libraries(csharpparser
  model)

target_compile_options(csharpparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS csharpparser DESTINATION ${INSTALL_PARSER_DIR})
