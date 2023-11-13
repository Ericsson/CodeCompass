cmake_minimum_required(VERSION 2.6)
project(CppTestProject)

# This is a dummy CMakeList that can be used to generate a build for the
# C++ test input files.

add_library(CppTestProject STATIC
  typedef.cpp
  cxxrecord.cpp
  enum.cpp
  function.cpp
  using.cpp
  variable.cpp
  namespace.cpp)
