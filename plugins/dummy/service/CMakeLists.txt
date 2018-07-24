set_rpath_with_libraries(ODB)

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

add_thrift_library(dummy "cpp" "")

add_library(dummyservice SHARED
  src/dummyservice.cpp
  src/plugin.cpp)

target_compile_options(dummyservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(dummyservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  ${ODB_LIBRARIES}
  dummythrift)

install(TARGETS dummyservice DESTINATION ${INSTALL_SERVICE_DIR})
