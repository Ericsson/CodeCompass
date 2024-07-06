cmake_minimum_required(VERSION 2.6)
project(CppMetricsTestProject)

add_library(CppMetricsTestProject STATIC
  mccabe.cpp
  lackofcohesion.cpp
  bumpyroad.cpp
  RelationalCohesion/OneTypeInModule/singleTypeInModule.cpp
  RelationalCohesion/MultipleTypesInModuleNoRelation/A.cpp
  RelationalCohesion/MultipleTypesInModuleNoRelation/B.cpp
  RelationalCohesion/MultipleTypesInModuleNoRelation/C.cpp
  RelationalCohesion/MultipleTypesInModuleNoRelation/D.cpp)
