include_directories(
  include
  gen-cpp
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/shared_constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/shared_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp
  COMMAND
    thrift --gen cpp --gen js -o ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/shared.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/shared.thrift
  COMMENT
    "Generate thrift for shared.thrift")

add_library(codecheckersharedthrift STATIC
  gen-cpp/shared_constants.cpp
  gen-cpp/shared_types.cpp)

target_compile_options(codecheckersharedthrift PUBLIC -fPIC)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/codeCheckerDBAccess_constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/codeCheckerDBAccess_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/codeCheckerDBAccess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp
  COMMAND
    thrift --gen cpp --gen js -o ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/codeCheckerDBAccess.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/codeCheckerDBAccess.thrift
  COMMENT
    "Generate thrift for codeCheckerDBAccess.thrift")

add_library(codecheckerthrift STATIC
  gen-cpp/codeCheckerDBAccess_constants.cpp
  gen-cpp/codeCheckerDBAccess_types.cpp
  gen-cpp/codeCheckerDBAccess.cpp)

target_compile_options(codecheckerthrift PUBLIC -fPIC)

add_dependencies(codecheckerthrift codecheckersharedthrift)

add_library(codecheckerservice SHARED
  src/codecheckerproxy.cpp
  src/plugin.cpp)

target_link_libraries(codecheckerservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  codecheckerthrift
  codecheckersharedthrift)

install(TARGETS codecheckerservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift(gen-js/)