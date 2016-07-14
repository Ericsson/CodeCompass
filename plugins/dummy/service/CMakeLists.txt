include_directories(
  include
  gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/mongoose/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginable-webserver/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginframework/include
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

list(APPEND SOURCES
  src/dummyservice.cpp
  src/plugin.cpp)

add_library(dummyservice SHARED ${SOURCES})
target_compile_options(dummyservice PUBLIC -Wno-unknown-pragmas)

set(_langs cpp java js)
set(_thriftFiles ${CMAKE_CURRENT_SOURCE_DIR}/dummy.thrift)
generate_thrift_files("${_thriftFiles}" "${_langs}" dummyservice)

install(TARGETS dummyservice DESTINATION ${INSTALL_SERVICE_DIR})