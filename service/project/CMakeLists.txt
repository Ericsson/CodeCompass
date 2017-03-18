include_directories(
  include
  gen-cpp
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS}
  ${ODB_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/common_constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/common_constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/common_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/common_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp
  COMMAND
    thrift --gen cpp --gen js --gen java -o ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/common.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/common.thrift
  COMMENT
    "Generate thrift for common.thrift")

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/project_constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/project_constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/project_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/project_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/ProjectService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/ProjectService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp
  COMMAND
    thrift --gen cpp --gen js --gen java -o ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/project.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/project.thrift
  COMMENT
    "Generate thrift for workspace.thrift")

add_library(commonthrift STATIC
  gen-cpp/common_constants.cpp
  gen-cpp/common_types.cpp)

target_compile_options(commonthrift PUBLIC -fPIC)

add_library(projectthrift STATIC
  gen-cpp/project_constants.cpp
  gen-cpp/project_types.cpp
  gen-cpp/ProjectService.cpp)

target_compile_options(projectthrift PUBLIC -fPIC)

add_dependencies(projectthrift commonthrift)

set(CMAKE_JAVA_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/lib/java/*)

add_jar(corethriftjava
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/core/Description.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/core/FilePosition.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/core/FileRange.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/core/InvalidId.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/core/InvalidInput.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/core/Position.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/core/Range.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/core/Timeout.java
  OUTPUT_NAME corethrift)

add_library(projectservice SHARED
  src/projectservice.cpp
  src/plugin.cpp)

target_link_libraries(projectservice
  util
  model
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  projectthrift
  commonthrift)

install(TARGETS projectservice DESTINATION ${INSTALL_SERVICE_DIR})
install_jar(corethriftjava "${INSTALL_JAVA_LIB_DIR}")
install_js_thrift(gen-js/)
