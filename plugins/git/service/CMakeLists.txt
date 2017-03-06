include_directories(
  include
  gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PLUGIN_DIR}/model/include
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/git_constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/git_constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/git_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/git_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/GitService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/GitService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp
  COMMAND
    thrift --gen cpp --gen js -o ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/git.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/git.thrift
  COMMENT
    "Generate thrift for git.thrift")

add_library(gitthrift STATIC
  gen-cpp/git_constants.cpp
  gen-cpp/git_types.cpp
  gen-cpp/GitService.cpp)

target_compile_options(gitthrift PUBLIC -fPIC)

add_library(gitservice SHARED
  src/plugin.cpp
  src/gitservice.cpp)

target_compile_options(gitservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(gitservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  gitthrift
  git2)

install(TARGETS gitservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift(gen-js/)