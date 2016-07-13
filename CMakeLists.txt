cmake_minimum_required(VERSION 2.8.9)
project(CodeCompass)

# Common config variables and settings
include(Config.cmake)

# Utility functions
include(Functions.cmake)

enable_testing()

find_package(Threads REQUIRED)
find_package(LLVM    REQUIRED CONFIG)
find_package(Boost   REQUIRED)
find_package(Odb     REQUIRED)
find_package(Thrift  REQUIRED)
find_package(GTest   REQUIRED)

add_subdirectory(model)
add_subdirectory(util)
add_subdirectory(parser)
add_subdirectory(plugins)