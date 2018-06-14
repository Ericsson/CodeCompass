include_directories(
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/model/include
  ${LLVM_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIRS})

include_directories(SYSTEM
  ${ODB_INCLUDE_DIRS})

add_library(util STATIC
  src/dbutil.cpp
  src/dynamiclibrary.cpp
  src/filesystem.cpp
  src/graph.cpp
  src/legendbuilder.cpp
  src/logutil.cpp
  src/parserutil.cpp
  src/pipedprocess.cpp
  src/util.cpp)

target_compile_options(util PUBLIC -fPIC)

find_boost_libraries(
  regex)
#FIXME: Why does util link unconditionally against sqlite3?
target_link_libraries(util
  ${Boost_LINK_LIBRARIES}
  sqlite3)
