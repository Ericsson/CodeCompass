cmake_minimum_required(VERSION 2.6)
project(CppMetricsTestProject)

add_library(CppMetricsTestProject STATIC
  functionmccabe.cpp
  typemccabe.cpp
  lackofcohesion.cpp
  bumpyroad.cpp
  afferentcoupling.cpp
  modulemetrics.cpp)
