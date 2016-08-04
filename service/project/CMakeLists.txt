include_directories(
  include
  common-api/gen-cpp
  project-api/gen-cpp
  result-api/gen-cpp
  ${PROJECT_SOURCE_DIR}/model/include
  ${compilationdatabase_PLUGIN_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/mongoose/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginable-webserver/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginframework/include
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

list(APPEND SOURCES
  src/projectservice.cpp
  src/plugin.cpp
)
    
set(GEN_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/common-api/gen-cpp/common_constants.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/common-api/gen-cpp/common_types.cpp  
  ${CMAKE_CURRENT_SOURCE_DIR}/project-api/gen-cpp/project_constants.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/project-api/gen-cpp/project_types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/project-api/gen-cpp/ProjectService.cpp  
  ${CMAKE_CURRENT_SOURCE_DIR}/result-api/gen-cpp/result_constants.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/result-api/gen-cpp/result_types.cpp)
  
set_source_files_properties(${GEN_SRC} PROPERTIES GENERATED TRUE)

add_library(coreservice SHARED ${SOURCES} ${GEN_SRC})
target_compile_options(coreservice PUBLIC -Wno-unknown-pragmas)

set(_langs cpp js java)
set(_thriftFiles
  ${CMAKE_CURRENT_SOURCE_DIR}/common.thrift
  ${CMAKE_CURRENT_SOURCE_DIR}/project.thrift
  ${CMAKE_CURRENT_SOURCE_DIR}/result.thrift)
generate_thrift_files("${_thriftFiles}" "${_langs}" coreservice)

target_link_libraries(coreservice
  model
  compilationdatabasemodel
  ${THRIFT_LIBTHRIFT_LIBRARIES}
)

install(TARGETS coreservice DESTINATION ${INSTALL_SERVICE_DIR})