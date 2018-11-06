include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/service/language/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PROJECT_SOURCE_DIR}/plugins/cpp/service/include
  ${PROJECT_SOURCE_DIR}/plugins/cpp/model/include)

include_directories(SYSTEM)

add_library(lspservice SHARED
  src/lspservice.cpp
  src/lsp_types.cpp
  src/plugin.cpp)

target_link_libraries(lspservice
  util)

add_dependencies(lspservice projectthrift)
add_dependencies(lspservice languagethrift)

install(TARGETS lspservice DESTINATION ${INSTALL_SERVICE_DIR})
