set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/lib/java/*)

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include)

add_library(javaparser SHARED
        src/javaparser.cpp)

target_compile_options(javaparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS javaparser DESTINATION ${INSTALL_PARSER_DIR})
