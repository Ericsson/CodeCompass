cmake_minimum_required(VERSION 2.8.9)
project(CodeCompass)

# Common config variables and settings
include(Config.cmake)

# Utility functions
include(Functions.cmake)

find_package(Threads REQUIRED)
find_package(LLVM    REQUIRED CONFIG)
find_package(Boost   REQUIRED)
find_package(Odb     REQUIRED)
find_package(Thrift  REQUIRED)

add_subdirectory(model)
add_subdirectory(util)
add_subdirectory(parser)
add_subdirectory(plugins)