include_directories(
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/model/include
  ${LLVM_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIRS})

add_library(util STATIC
  src/dbutil.cpp
  src/parserutil.cpp
  src/webserverutil.cpp
  src/dynamiclibrary.cpp
  src/logutil.cpp
  src/graph.cpp
  src/legendbuilder.cpp
  src/util.cpp
  src/pipedprocess.cpp)

target_compile_options(util PUBLIC -fPIC)
target_link_libraries(util
  boost_regex
  sqlite3)
