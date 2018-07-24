map_model_include_directories(${PROJECT_SOURCE_DIR}/model/include)
include_directories(${ODB_MODEL_INCLUDE_DIRECTORIES})

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${ODB_INCLUDE_DIRS})

add_thrift_library(common "cpp;js;java" "")

set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*)

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

add_thrift_library(project "cpp;js;java" "common")

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
