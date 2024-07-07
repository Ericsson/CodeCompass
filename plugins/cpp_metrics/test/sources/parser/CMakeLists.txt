cmake_minimum_required(VERSION 2.6)
project(CppMetricsTestProject)

file(GLOB_RECURSE RELATIONAL_COHESION_SOURCES "RelationalCohesion/**/*.cpp")

add_library(CppMetricsTestProject STATIC
  mccabe.cpp
  lackofcohesion.cpp
  bumpyroad.cpp
  ${RELATIONAL_COHESION_SOURCES})
