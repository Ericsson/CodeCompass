include_directories(
  include
  ${ODB_INCLUDE_DIRS}
)

set(ODB_SOURCES
  include/model/file.h
  include/model/filecontent.h
  include/model/fileloc.h
  include/model/option.h
  include/model/position.h
  include/model/statistics.h  
)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(model)