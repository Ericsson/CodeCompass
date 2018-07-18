include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include)

add_library(gitparser SHARED 
  src/gitparser.cpp)

target_link_libraries(gitparser
  util
  git2
  ssl)

install(TARGETS gitparser DESTINATION ${INSTALL_PARSER_DIR})
