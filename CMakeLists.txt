cmake_minimum_required(VERSION 3.4.3)
project(CodeCompass)

# Common config variables and settings
include(Config.cmake)

# Utility functions
include(Functions.cmake)

find_package(GTest)
enable_testing()

find_package(Boost   REQUIRED
  COMPONENTS filesystem log program_options regex system thread)
find_package(Java    REQUIRED)
find_package(Odb     REQUIRED)
find_package(Thrift  REQUIRED)
find_package(Threads REQUIRED)

include(UseJava)

# Functions that are used to set up installation properly for the environment
# in which CodeCompass will be run
include(Exports.cmake)

add_subdirectory(logger)
add_subdirectory(model)
add_subdirectory(parser)
add_subdirectory(service)
add_subdirectory(scripts)
add_subdirectory(util)
add_subdirectory(webgui)
add_subdirectory(webserver)

add_subdirectory(plugins)

# Install java libraries
install(DIRECTORY
  lib/java/
  DESTINATION "${INSTALL_JAVA_LIB_DIR}")
