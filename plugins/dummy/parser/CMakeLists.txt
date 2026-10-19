include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include)

include_directories(SYSTEM
  ${Boost_INCLUDE_DIRS})

add_library(dummyparser SHARED
  src/dummyparser.cpp)

target_compile_options(dummyparser PUBLIC -Wno-unknown-pragmas)

target_link_libraries(dummyparser
  ${Boost_LIBRARIES})

install(TARGETS dummyparser DESTINATION ${INSTALL_PARSER_DIR})
