cmake_minimum_required(VERSION 3.4.3)
project(CodeCompass)

# Common config variables and settings
include(Config.cmake)

# Utility functions
include(Functions.cmake)

enable_testing()

find_package(Threads REQUIRED)
find_package(LLVM    REQUIRED CONFIG)
find_package(Boost   REQUIRED COMPONENTS filesystem log program_options regex system thread)
find_package(Odb     REQUIRED)
find_package(Thrift  REQUIRED)
find_package(Java    REQUIRED)
find_package(GTest)

include(UseJava)

# Set the third-party libraries' path properly so the installation can find them.
include(Exports.cmake)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${RUNENV_LD_LIBRARY_PATH}")

add_subdirectory(model)
add_subdirectory(util)
add_subdirectory(parser)
add_subdirectory(plugins)
add_subdirectory(webserver)
add_subdirectory(service)
add_subdirectory(logger)
add_subdirectory(scripts)
add_subdirectory(webgui)

# Install java libraries
install(DIRECTORY
  lib/java/
  DESTINATION "${INSTALL_JAVA_LIB_DIR}")

# Install Clang additional files
install(DIRECTORY
  ${LLVM_LIBRARY_DIRS}/clang
  DESTINATION "${INSTALL_LIB_DIR}")
