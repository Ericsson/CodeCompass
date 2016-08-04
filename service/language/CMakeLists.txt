include_directories(
  include
  language-api/gen-cpp
  ${PROJECT_SOURCE_DIR}/model/include
  ${cpp_PLUGIN_DIR}/model/include
  ${compilationdatabase_PLUGIN_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/service/project/common-api/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/project-api/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/result-api/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/mongoose/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginable-webserver/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginframework/include
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

list(APPEND SOURCES
  src/languageservice.cpp
)

set(GEN_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/language-api/gen-cpp/language_constants.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/language-api/gen-cpp/language_types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/language-api/gen-cpp/LanguageService.cpp)
  
set_source_files_properties(${GEN_SRC} PROPERTIES GENERATED TRUE)

add_library(languageservice SHARED ${SOURCES} ${GEN_SRC})
target_compile_options(languageservice PUBLIC -Wno-unknown-pragmas)

set(_langs cpp js)
set(_thriftFiles ${CMAKE_CURRENT_SOURCE_DIR}/language.thrift)
generate_thrift_files(
  "${_thriftFiles}"
  "${_langs}"
  workspaceservice
  -I ${PROJECT_SOURCE_DIR}/service )

target_link_libraries(languageservice
  model
  cppmodel
  compilationdatabasemodel
  coreservice 
  ${THRIFT_LIBTHRIFT_LIBRARIES}
)

install(TARGETS languageservice DESTINATION ${INSTALL_SERVICE_DIR})