map_model_include_directories(${PROJECT_SOURCE_DIR}/model/include)

include_directories(
  ${PROJECT_SOURCE_DIR}/util/include
  ${BOOST_INCLUDE_DIRS}
  ${ODB_MODEL_INCLUDE_DIRECTORIES})

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

find_boost_libraries(regex)

target_link_libraries(util
  ${Boost_LINK_LIBRARIES})

string(TOLOWER "${DATABASE}" _database)
if (${_database} STREQUAL "sqlite")
  target_link_libraries(util
    sqlite3)
endif()
