include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

set(AUTHENTICATIONTHRIFT_OUTPUT_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/authentication_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/authentication_constants.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/authentication_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/authentication_types.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/AuthenticationService.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/AuthenticationService.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-js/authentication_types.js
  ${CMAKE_CURRENT_BINARY_DIR}/gen-js/AuthenticationService.js)

set(AUTHENTICATIONTHRIFT_THRIFT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/authentication.thrift)

set(AUTHENTICATIONTHRIFT_ADDITIONAL_ARGS --gen cpp --gen js:jquery)

generate_from_thrift("${AUTHENTICATIONTHRIFT_OUTPUT_FILES}" "${AUTHENTICATIONTHRIFT_THRIFT_FILE}" "${AUTHENTICATIONTHRIFT_ADDITIONAL_ARGS}")

set(authenticationthrift_sources
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/authentication_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/authentication_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/AuthenticationService.cpp)

add_library(authenticationthrift STATIC ${authenticationthrift_sources})

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
