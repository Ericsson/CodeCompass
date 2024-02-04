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
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PROJECT_SOURCE_DIR}/service/lsp/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_library(cpplspservice SHARED
  src/cpplspservice.cpp
  src/plugin.cpp)

target_link_libraries(cpplspservice
  cppservice
  lspservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES})

install(TARGETS cpplspservice DESTINATION ${INSTALL_SERVICE_DIR})
