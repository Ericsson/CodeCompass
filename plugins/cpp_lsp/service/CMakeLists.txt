include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_BINARY_DIR}/service/language/gen-cpp
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PROJECT_SOURCE_DIR}/plugins/cpp/service/include
  ${PROJECT_SOURCE_DIR}/plugins/cpp/model/include
  ${CMAKE_BINARY_DIR}/model/include
  ${CMAKE_BINARY_DIR}/plugins/cpp/model/include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/lsp_constants.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/lsp_constants.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/lsp_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/lsp_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CppLspService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CppLspService.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp
      -o ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/lsp.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/lsp.thrift
  COMMENT
    "Generating Thrift for lsp.thrift")

add_library(lspthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/lsp_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/lsp_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/LspService.cpp)

target_compile_options(lspthrift PUBLIC -fPIC)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_library(cpplspservice SHARED
  src/cpplspservice.cpp
  src/lsp_types.cpp
  src/plugin.cpp)

target_link_libraries(cpplspservice
  util
  lspthrift
  ${THRIFT_LIBTHRIFT_LIBRARIES})

add_dependencies(cpplspservice projectthrift)
add_dependencies(cpplspservice languagethrift)

install(TARGETS cpplspservice DESTINATION ${INSTALL_SERVICE_DIR})