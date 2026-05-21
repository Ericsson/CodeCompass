include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${Boost_INCLUDE_DIRS}
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS}
  ${ODB_INCLUDE_DIRS})

# The folders here are added so that executing `make clean` removes all
# generated files.
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/common_constants.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/common_constants.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/common_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/common_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-java
    ${CMAKE_CURRENT_BINARY_DIR}/gen-js
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen js --gen java
      -o ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/common.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/common.thrift
  COMMENT
    "Generating Thrift for common.thrift")

# Do not add the same folders to both output commands as it produces a warning.
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/project_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/project_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/ProjectService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/ProjectService.h
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen js --gen java
      -o ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/project.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/project.thrift
  COMMENT
    "Generating Thrift for project.thrift")

add_library(commonthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/common_types.cpp)

target_compile_options(commonthrift PUBLIC -fPIC)

add_library(projectthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/project_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/ProjectService.cpp)

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

if(APPLE)
  # Use Linux-like linking behaviour, dynamic libraries may contain references without implementation.
  # projectservice references headers from the webserver without implementation
  set_target_properties(projectservice PROPERTIES LINK_FLAGS
        "-undefined dynamic_lookup")
endif(APPLE)

install(TARGETS projectservice DESTINATION ${INSTALL_SERVICE_DIR})
install_jar(corethriftjava "${INSTALL_JAVA_LIB_DIR}")
install_js_thrift()
