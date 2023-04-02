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
  ${PROJECT_SOURCE_DIR}/webserver/include)

add_library(cpplspservice SHARED
  src/cpplspservice.cpp
  src/lsp_types.cpp
  src/plugin.cpp)

target_link_libraries(cpplspservice
  util)

add_dependencies(cpplspservice projectthrift)
add_dependencies(cpplspservice languagethrift)

install(TARGETS cpplspservice DESTINATION ${INSTALL_SERVICE_DIR})