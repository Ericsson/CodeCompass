include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS}
  ${ODB_INCLUDE_DIRS})

# The folders here are added so that executing `make clean` removes all
# generated files.

set(COMMONTHRIFT_OUTPUT_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/common_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/common_constants.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/common_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/common_types.h)

set(COMMONTHRIFT_THRIFT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/common.thrift)

set(COMMONTHRIFT_ADDITIONAL_ARGS --gen cpp --gen js --gen java)

generate_from_thrift("${COMMONTHRIFT_OUTPUT_FILES}" "${COMMONTHRIFT_THRIFT_FILE}" "${COMMONTHRIFT_ADDITIONAL_ARGS}")

set(commonthrift_sources
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/common_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/common_types.cpp)

add_library(commonthrift STATIC ${commonthrift_sources})

target_compile_options(commonthrift PUBLIC -fPIC)

set(PROJECTTHRIFT_OUTPUT_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/project_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/project_constants.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/project_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/project_types.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/ProjectService.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/ProjectService.h)

set(PROJECTTHRIFT_THRIFT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/project.thrift)

set(PROJECTTHRIFT_ADDITIONAL_ARGS --gen cpp --gen js --gen java)

generate_from_thrift("${PROJECTTHRIFT_OUTPUT_FILES}" "${PROJECTTHRIFT_THRIFT_FILE}" "${PROJECTTHRIFT_ADDITIONAL_ARGS}")

set(projectthrift_sources
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/project_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/project_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/ProjectService.cpp)

add_library(projectthrift STATIC ${projectthrift_sources})

target_compile_options(projectthrift PUBLIC -fPIC)

add_dependencies(projectthrift commonthrift)

set(CMAKE_JAVA_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/lib/java/*)

add_jar(corethriftjava
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/core/Description.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/core/FilePosition.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/core/FileRange.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/core/InvalidId.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/core/InvalidInput.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/core/Position.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/core/Range.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/core/Timeout.java
  OUTPUT_NAME corethrift)

add_dependencies(corethriftjava commonthrift)

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
install_js_thrift()
