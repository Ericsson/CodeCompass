include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/authentication_constants.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/authentication_constants.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/authentication_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/authentication_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/AuthenticationService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/AuthenticationService.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-js/authentication_types.js
    ${CMAKE_CURRENT_BINARY_DIR}/gen-js/AuthenticationService.js
    ${CMAKE_CURRENT_BINARY_DIR}/gen-js
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen js:jquery --gen js:ts
      -o ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/authentication.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/authentication.thrift
  COMMENT
    "Generating Thrift for authentication.thrift")

add_library(authenticationthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/authentication_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/authentication_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/AuthenticationService.cpp)

target_compile_options(authenticationthrift PUBLIC -fPIC -Wno-unknown-pragmas)

add_library(authenticationservice SHARED
  src/authenticationservice.cpp
  src/plugin.cpp)

target_link_libraries(authenticationservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  authenticationthrift)

install(TARGETS authenticationservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift()
