include_directories(
  include)

include_directories(SYSTEM
  ${ODB_INCLUDE_DIRS})

set(ODB_SOURCES
  include/model/buildaction.h
  include/model/buildlog.h
  include/model/buildsourcetarget.h
  include/model/filecontent.h
  include/model/file.h
  include/model/fileloc.h
  include/model/position.h
  include/model/statistics.h)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(model ${ODB_CXX_SOURCES})

install_sql(${CMAKE_CURRENT_BINARY_DIR})
