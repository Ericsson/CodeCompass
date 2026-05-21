include_directories(
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/model/include)

include_directories(SYSTEM
  ${Boost_INCLUDE_DIRS}
  ${ODB_INCLUDE_DIRS}
  ${GRAPHVIZ_INCLUDE_DIRS})

add_library(util SHARED
  src/dbutil.cpp
  src/dynamiclibrary.cpp
  src/filesystem.cpp
  src/graph.cpp
  src/legendbuilder.cpp
  src/logutil.cpp
  src/parserutil.cpp
  src/pipedprocess.cpp
  src/util.cpp)

target_link_libraries(util
  model
  ${Boost_LIBRARIES}
  ${ODB_LIBRARIES}
  ${GRAPHVIZ_GVC_LIBRARY}
  ${GRAPHVIZ_CGRAPH_LIBRARY})

string(TOLOWER "${DATABASE}" _database)
if (${_database} STREQUAL "sqlite")
  find_package(SQLite3)
  target_link_libraries(util
    ${SQLite3_LIBRARIES})
endif()

install(TARGETS util DESTINATION ${INSTALL_LIB_DIR})
