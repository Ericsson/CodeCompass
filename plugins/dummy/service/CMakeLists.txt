include_directories(
  include
  gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/mongoose/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginable-webserver/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginframework/include
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

set(_THRIFT_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/DummyService.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/DummyService.h
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/dummy_constants.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/dummy_constants.h
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/dummy_types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/dummy_types.h)

add_custom_command(
  OUTPUT
    ${_THRIFT_SRC}
  COMMAND
    thrift --gen cpp -o ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dummy.thrift
#  COMMAND
#    cp -f ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/[dD]ummy*.h ${CMAKE_CURRENT_SOURCE_DIR}/include/service
#  COMMAND
#    cp -f ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/[dD]ummy*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src
  COMMAND
    thrift --gen js -o ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dummy.thrift
  COMMAND
    thrift --gen java -o ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dummy.thrift
  COMMAND
    thrift --gen py -o ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dummy.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/dummy.thrift
  COMMENT "Generate thrift for dummy.thrift")

list(APPEND SOURCES
  src/dummyservice.cpp
  src/plugin.cpp
  ${_THRIFT_SRC})
    
add_library(dummyservice SHARED ${SOURCES})
target_compile_options(dummyservice PUBLIC -Wno-unknown-pragmas)
target_link_libraries(dummyservice
  ${THRIFT_LIBTHRIFT_LIBRARIES}    
)

install(TARGETS dummyservice DESTINATION ${INSTALL_SERVICE_DIR})