cmake_minimum_required(VERSION 2.6)
project(CppMetricsTestProject)

add_library(CppMetricsTestProject STATIC
  mccabe.cpp
  lackofcohesion.cpp)
