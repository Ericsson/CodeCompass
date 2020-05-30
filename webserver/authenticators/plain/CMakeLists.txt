include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

add_library(plainauth SHARED
  src/plugin.cpp)

target_compile_options(plainauth PUBLIC -Wno-unknown-pragmas)

target_link_libraries(plainauth
  util)

install(TARGETS plainauth DESTINATION ${INSTALL_AUTH_DIR})
