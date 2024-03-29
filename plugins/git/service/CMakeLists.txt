include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PLUGIN_DIR}/model/include
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PLUGIN_DIR}/model/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/git_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/git_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/GitService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/GitService.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-js
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen js
      -o ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/git.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/git.thrift
  COMMENT
    "Generating Thrift for git.thrift")

add_library(gitthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/git_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/GitService.cpp)

target_compile_options(gitthrift PUBLIC -fPIC)

add_library(gitservice SHARED
  src/plugin.cpp
  src/gitservice.cpp)

target_compile_options(gitservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(gitservice
  util
  projectservice
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  ${ODB_LIBRARIES}
  gitthrift
  git2)

install(TARGETS gitservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift()
