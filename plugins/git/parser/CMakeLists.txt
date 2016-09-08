include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include )

set(SOURCES
  src/gitparser.cpp)

add_library(gitparser SHARED ${SOURCES})
target_link_libraries(gitparser
  util
  git2
  ssl
  crypto)

install(TARGETS gitparser DESTINATION ${INSTALL_PARSER_DIR})
