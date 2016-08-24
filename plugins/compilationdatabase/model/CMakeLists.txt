include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${ODB_INCLUDE_DIRS})

set(ODB_SOURCES
  include/model/buildsourcetarget.h
  include/model/buildaction.h
  include/model/buildlog.h)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(compilationdatabasemodel ${ODB_CXX_SOURCES})
target_link_libraries(compilationdatabasemodel
  model)
