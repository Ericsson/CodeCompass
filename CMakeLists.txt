cmake_minimum_required(VERSION 3.5.1)
project(CodeCompass)

# Common config variables and settings
include(Config.cmake)

# Utility functions
include(Functions.cmake)

# Do some sanity check on the testing setup and enable testing if applicable.
include(Testing.cmake)

find_package(Boost   REQUIRED COMPONENTS filesystem log program_options regex system thread python)
find_package(Java    REQUIRED)
find_package(Odb     REQUIRED)
find_package(Threads REQUIRED)
find_package(Thrift  REQUIRED)
find_package(GTest)

include(UseJava)
set(CMAKE_JAVA_COMPILE_FLAGS -encoding utf8)

# Set the third-party libraries' path properly so the installation can find them.
include(Exports.cmake)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${RUNENV_LD_LIBRARY_PATH}")

# Modules
add_subdirectory(logger)
add_subdirectory(model)
add_subdirectory(parser)
add_subdirectory(scripts)
add_subdirectory(service)
add_subdirectory(util)
add_subdirectory(plugins) # must precede webgui
add_subdirectory(webgui)
add_subdirectory(webserver)

# Install java libraries
install(DIRECTORY
  lib/java/
  DESTINATION "${INSTALL_JAVA_LIB_DIR}")
